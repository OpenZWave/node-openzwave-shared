/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
* Copyright (c) 2015-2017 Elias Karakoulakis <elias.karakoulakis@gmail.com>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "openzwave.hpp"
#include "Notification.h"

using namespace v8;
using namespace node;
using namespace OpenZWave;

namespace OZW {

  // check
	bool checkType(bool predicate) {
		if (!predicate) {
			Nan::ThrowTypeError("wrong value type!");
		}
		return predicate;
	}

	/*
	* Return the node for this request.
	*/
	NodeInfo *get_node_info(uint8 nodeid) {
		mutex::scoped_lock sl(znodes_mutex);
		if (znodes.find(nodeid) != znodes.end()) {
			return znodes[nodeid];
		};
		return NULL;
	}

	/*
	* Delete NodeInfo after a NodeReset or NodeRemoved notification
	*/
	void delete_node(uint8 nodeid) {
		mutex::scoped_lock sl(znodes_mutex);
		if (znodes.erase(nodeid)) {
			// TODO: extra cleanup??
		}
	}

	SceneInfo *get_scene_info(uint8 sceneid) {
		::std::list<SceneInfo *>::iterator it;

		SceneInfo *scene;

		for (it = zscenes.begin(); it != zscenes.end(); ++it) {
			scene = *it;
			if (scene->sceneid == sceneid)
				return scene;
		}

		return NULL;
	}

	::std::string getValueIdDescriptor(OpenZWave::ValueID value) {
		char buffer[32];
		snprintf(buffer, 32, "%d-%d-%d-%d", value.GetNodeId(), value.GetCommandClassId(), value.GetInstance(), value.GetIndex());
		return ::std::string(buffer);
	}
	::std::string getValueIdDescriptor(uint8 node_id, uint8 class_id, OZWValueIdIndex instance, uint8 index) {
		char buffer[32];
		snprintf(buffer, 32, "%d-%d-%d-%d", node_id, class_id, instance, index);
		return ::std::string(buffer);
	}

	// populate a v8 object with an attribute called 'value' whose value is the
	// ZWave value, as returned from its proper typed call.
	void setValObj(Local<Object>&valobj, OpenZWave::ValueID &value) {
		/*
		* The value itself is type-specific.
		*/
		switch (value.GetType()) {
			case OpenZWave::ValueID::ValueType_Bool: {
				bool val;
				OZWManager( GetValueAsBool, value, &val);
				AddBooleanProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_Byte: {
				uint8 val;
				OZWManager( GetValueAsByte, value, &val);
				AddIntegerProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_Decimal: {
				::std::string val;
				OZWManager( GetValueAsString, value, &val);
				AddStringProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_Int: {
				int32 val;
				OZWManager( GetValueAsInt, value, &val);
				AddIntegerProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_List: {
 				::std::string val;
				::std::vector < ::std::string > items;
				// populate array of all available items in the list
				OZWManager( GetValueListItems, value, &items);
				AddArrayOfStringProp(valobj, values, items);
				// populated selected element
				OZWManager( GetValueListSelection, value, &val);
				AddStringProp(valobj, value, val.c_str())
 				break;
			}
			case OpenZWave::ValueID::ValueType_Short: {
				int16 val;
				OZWManager( GetValueAsShort, value, &val);
				AddIntegerProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_String: {
				::std::string val;
				OZWManager( GetValueAsString, value, &val);
				AddStringProp(valobj, value, val.c_str())
				break;
			}
#if OPENZWAVE_16
#define SET(val,offset)   val |=  (1 << offset)
#define CLEAR(val,offset) val &= ~(1 << offset)
			// just return the whole bitset, mask your bit in JS land
			case OpenZWave::ValueID::ValueType_BitSet: {
				uint8 val = 0;
				bool bit;
				for (uint8 pos = 0; pos < 8; pos++) {
					OZWManager( GetValueAsBitSet, value, pos, &bit);
					bit ? SET(val, pos) : CLEAR(val, pos);
				}
				AddIntegerProp(valobj, value, val);
			}
#endif
			/*
			* Buttons do not have a value.
			*/
			case OpenZWave::ValueID::ValueType_Button: {
				break;
			}
			case OpenZWave::ValueID::ValueType_Schedule: {
				break;
			}
			case OpenZWave::ValueID::ValueType_Raw: {
				uint8 *val, len;
				OZWManager( GetValueAsRaw, value, &val, &len);
				Nan::Set(valobj,
					Nan::New<String>("value").ToLocalChecked(),
					Nan::CopyBuffer((char *)val, len).ToLocalChecked()
				);
				delete [] val;
				break;
			}
			default: {
				fprintf(stderr, "unsupported value type: 0x%x\n", value.GetType());
				break;
			}
		}
	}

#if OPENZWAVE_SCENES
	// populate a v8 object with an attribute called 'value' whose value is the
	// SCENE value (not the current one!) - as returned from its proper typed call
	// (using Manager::SceneGetValueAsXXX calls)
	void setSceneValObj(uint8 sceneid, Local<Object>&valobj, OpenZWave::ValueID &value) {
		/*
		* The value itself is type-specific.
		*/
		switch (value.GetType()) {
			case OpenZWave::ValueID::ValueType_Bool: {
				bool val;
				OZWManager( SceneGetValueAsBool, sceneid, value, &val);
				AddBooleanProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_Byte: {
				uint8 val;
				OZWManager( SceneGetValueAsByte, sceneid, value, &val);
				AddIntegerProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_Decimal: {
				::std::string val;
				OZWManager( SceneGetValueAsString, sceneid, value, &val);
				AddStringProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_List: {
				::std::string val;
				::std::vector < ::std::string > items;
				// populate array of all available items in the list
				OZWManager( GetValueListItems, value, &items);
				AddArrayOfStringProp(valobj, values, items);
				// populated selected element
				OZWManager( SceneGetValueAsString, sceneid, value, &val);
				AddStringProp(valobj, value, val.c_str())
				break;
			}
			case OpenZWave::ValueID::ValueType_Int: {
				int32 val;
				OZWManager( SceneGetValueAsInt, sceneid, value, &val);
				AddIntegerProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_Short: {
				int16 val;
				OZWManager( SceneGetValueAsShort, sceneid, value, &val);
				AddIntegerProp(valobj, value, val);
				break;
			}
			case OpenZWave::ValueID::ValueType_String: {
				::std::string val;
				OZWManager( SceneGetValueAsString, sceneid, value, &val);
				AddStringProp(valobj, value, val.c_str())
				break;
			}
			/*
			* Buttons, Schedules, Lists and Raw do not have a SceneGetValue extractor.
			*/
			case OpenZWave::ValueID::ValueType_Button:
			case OpenZWave::ValueID::ValueType_Schedule:
			case OpenZWave::ValueID::ValueType_Raw: {
				fprintf(stderr, "unsupported scene value type: 0x%x\n", value.GetType());
				break;
			}
		}
	}
#endif

	// populate a v8 Object with useful information about a ZWave node
	void populateNode(
		v8::Local<v8::Object>& nodeobj,
		uint32 homeid, uint8 nodeid
	) {
		OpenZWave::Manager *mgr = OpenZWave::Manager::Get();
		AddStringProp(nodeobj, manufacturer, mgr->GetNodeManufacturerName(homeid, nodeid).c_str());
		AddStringProp(nodeobj, manufacturerid,   mgr->GetNodeManufacturerId(homeid, nodeid).c_str());
		AddStringProp(nodeobj, product,    mgr->GetNodeProductName(homeid, nodeid).c_str());
		AddStringProp(nodeobj, producttype,mgr->GetNodeProductType(homeid, nodeid).c_str());
		AddStringProp(nodeobj, productid,  mgr->GetNodeProductId(homeid, nodeid).c_str());
		AddStringProp(nodeobj, type, mgr->GetNodeType(homeid, nodeid).c_str());
		AddStringProp(nodeobj, name, mgr->GetNodeName(homeid, nodeid).c_str());
		AddStringProp(nodeobj, loc,  mgr->GetNodeLocation(homeid, nodeid).c_str());
	}

	void populateValueId(v8::Local<v8::Object>& nodeobj, OpenZWave::ValueID value) {
		Nan::EscapableHandleScope handle_scope;
		OpenZWave::Manager *mgr = OpenZWave::Manager::Get();
		::std::string buffer = getValueIdDescriptor(value);
		/*
		* Common value types.
		*/
		// no 64-bit ints in Javascript:
		//Nan::Set(valobj, Nan::New<String>("id"), Nan::New<Integer>(value.GetId()));
		//Nan::Set(valobj, Nan::New<String>("change_verified").ToLocalChecked(), Nan::New<Boolean>(mgr->GetChangeVerified(value))->ToBoolean());
		//
		AddStringProp(nodeobj,  value_id,  buffer.c_str());
		AddIntegerProp(nodeobj, node_id,   value.GetNodeId());
		AddIntegerProp(nodeobj, class_id,  value.GetCommandClassId());
#if OPENZWAVE_VALUETYPE_FROM_VALUEID
		AddStringProp (nodeobj, type,      value.GetTypeAsString());
		AddStringProp (nodeobj, genre,     value.GetGenreAsString());
#elif OPENZWAVE_VALUETYPE_FROM_ENUM
		AddStringProp (nodeobj, type,      OpenZWave::Internal::VC::ValueList::GetTypeNameFromEnum(value.GetType()));
		AddStringProp (nodeobj, genre,     OpenZWave::Internal::VC::ValueList::GetGenreNameFromEnum(value.GetGenre()));
#else
		AddIntegerProp(nodeobj, type,      value.GetType());
		AddIntegerProp(nodeobj, genre,     value.GetGenre());
#endif
		AddIntegerProp(nodeobj, instance,  value.GetInstance());
		AddIntegerProp(nodeobj, index,     value.GetIndex());
		AddStringProp (nodeobj, label,     mgr->GetValueLabel(value).c_str());
		AddStringProp (nodeobj, units,     mgr->GetValueUnits(value).c_str());
		AddStringProp (nodeobj, help,      mgr->GetValueHelp(value).c_str());
		AddBooleanProp(nodeobj, read_only, mgr->IsValueReadOnly(value));
		AddBooleanProp(nodeobj, write_only,mgr->IsValueWriteOnly(value));
		AddIntegerProp(nodeobj, min,       mgr->GetValueMin(value));
		AddIntegerProp(nodeobj, max,       mgr->GetValueMax(value));
		AddBooleanProp(nodeobj, is_polled, mgr->IsValuePolled(value));
	}

	// create a V8 object from a OpenZWave::ValueID
	Local<Object> zwaveValue2v8Value(OpenZWave::ValueID value) {
		Nan::EscapableHandleScope handle_scope;
		Local <Object> valobj = Nan::New<Object>();
		populateValueId(valobj, value);
		setValObj(valobj, value);
		return handle_scope.Escape(valobj);
	}

#ifdef OPENZWAVE_DEPRECATED16
	// create a V8 object from a ZWave scene value
	Local<Object> zwaveSceneValue2v8Value(uint8 sceneId, OpenZWave::ValueID value) {
		Nan::EscapableHandleScope handle_scope;
		Local <Object> valobj =  Nan::New<Object>();
		populateValueId(valobj, value);
		setSceneValObj(sceneId, valobj, value);
		return handle_scope.Escape(valobj);
	}
#endif

	bool isOzwValue(Local<Object>& o) {
		return (Nan::HasOwnProperty(o, Nan::New<String>("node_id").ToLocalChecked()).FromJust()
			&& Nan::HasOwnProperty(o, Nan::New<String>("class_id").ToLocalChecked()).FromJust()
			&& Nan::HasOwnProperty(o, Nan::New<String>("instance").ToLocalChecked()).FromJust()
			&& Nan::HasOwnProperty(o, Nan::New<String>("index").ToLocalChecked()).FromJust()
		);
	}

  /* get the ZWave ValueID from the arguments passed to a node.js function,
		which can be either:
	1) a series of 4 arguments of the basic value constituents (legacy mode) or
	2) a single Javascript object (such as the one returned from zwaveValue2v8Value() )
	In both cases the following args are needed:
		nodeid : the ZWave Node ID
		class_id: the command class ID
		instance: the instance of the command (usually 1)
		index: the index of the command (usually 0)
*/
	OpenZWave::ValueID* populateValueId(const Nan::FunctionCallbackInfo<v8::Value> &info, uint8 offset) {
		uint8 nodeid, comclass, instance;
		OZWValueIdIndex index;
		if ( (info.Length() >= offset) && info[offset]->IsObject() ) {
			Local<Object> o = Nan::To<Object>(info[offset]).ToLocalChecked();
			if (isOzwValue(o)) {
				nodeid   = Nan::To<Number>(Nan::Get(o, Nan::New<String>("node_id").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value();
				comclass = Nan::To<Number>(Nan::Get(o, Nan::New<String>("class_id").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value();
				instance = Nan::To<Number>(Nan::Get(o, Nan::New<String>("instance").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value();
				index    = Nan::To<Number>(Nan::Get(o, Nan::New<String>("index").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value();
			} else {
				::std::string errmsg("OpenZWave valueId object not found: ");
				Nan::JSON NanJSON;
				Nan::MaybeLocal<v8::String> result = NanJSON.Stringify(o);
				if (!result.IsEmpty()) {
  				v8::Local<v8::String> stringified = result.ToLocalChecked();
					errmsg += *Nan::Utf8String(stringified);
				}
				Nan::ThrowTypeError(errmsg.c_str());
				return (NULL);
			}
		} else if ((info.Length() >= offset+4)) {
			// legacy mode
			nodeid   = Nan::To<Number>(info[offset]).ToLocalChecked()->Value();
			comclass = Nan::To<Number>(info[offset+1]).ToLocalChecked()->Value();
			instance = Nan::To<Number>(info[offset+2]).ToLocalChecked()->Value();
			index    = Nan::To<Number>(info[offset+3]).ToLocalChecked()->Value();
		} else {
			::std::string errmsg("OpenZWave valueId not found. Pass either a JS object with {node_id, class_id, instance, index} or the raw values in this order.");
			Nan::ThrowTypeError(errmsg.c_str());
			return (NULL);
		}

		NodeInfo *node = NULL;
		::std::list<OpenZWave::ValueID>::iterator vit;

		if ((node = get_node_info(nodeid))) {
			for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
				if (((*vit).GetCommandClassId() == comclass) && ((*vit).GetInstance() == instance) && ((*vit).GetIndex() == index)) {
					return ( &*vit );
				}
			}
		}
		::std::string errmsg(
			::std::string("OpenZWave valueId not found: ") +
			getValueIdDescriptor(nodeid, comclass, instance, index));
		Nan::ThrowTypeError(errmsg.c_str());
		return( NULL );
	}

const char* getControllerStateAsStr (OpenZWave::Driver::ControllerState _state) {
	switch (_state) {
	case OpenZWave::Driver::ControllerState_Normal: 		return "Normal";
	case OpenZWave::Driver::ControllerState_Starting:   return "Starting";
	case OpenZWave::Driver::ControllerState_Cancel:		  return "Cancel";
	case OpenZWave::Driver::ControllerState_Error:      return "Error";
	case OpenZWave::Driver::ControllerState_Waiting: 	  return "Waiting";
	case OpenZWave::Driver::ControllerState_Sleeping:	  return "Sleeping";
	case OpenZWave::Driver::ControllerState_InProgress: return "In Progress";
	case OpenZWave::Driver::ControllerState_Completed:  return "Completed";
	case OpenZWave::Driver::ControllerState_Failed:     return "Failed";
	case OpenZWave::Driver::ControllerState_NodeOK:     return "Node OK";
	case OpenZWave::Driver::ControllerState_NodeFailed: return "Node Failed";
	}
	return "";
}

const char* getControllerErrorAsStr(OpenZWave::Driver::ControllerError _err) {
	switch (_err) {
		case OpenZWave::Driver::ControllerError_None:           return "None";
		case OpenZWave::Driver::ControllerError_ButtonNotFound: return "Button not found";
		case OpenZWave::Driver::ControllerError_NodeNotFound:   return "Node not found";
		case OpenZWave::Driver::ControllerError_NotBridge:      return "Not bridge";
		case OpenZWave::Driver::ControllerError_NotSUC:					return "Not SUC";
		case OpenZWave::Driver::ControllerError_NotSecondary:   return "Not secondary";
		case OpenZWave::Driver::ControllerError_NotPrimary:     return "Not primary";
		case OpenZWave::Driver::ControllerError_IsPrimary:      return "Is primary";
		case OpenZWave::Driver::ControllerError_NotFound:       return "Not found";
		case OpenZWave::Driver::ControllerError_Busy:           return "Busy";
		case OpenZWave::Driver::ControllerError_Failed:         return "Failed";
		case OpenZWave::Driver::ControllerError_Disabled:       return "Disabled";
		case OpenZWave::Driver::ControllerError_Overflow:       return "Overflow";
	}
	return "";
}

// backport code from OpenZWave to get notification help message
const ::std::string getNotifHelpMsg(Notification const *n) {
#if OPENZWAVE_SECURITY == 1
		return n->GetAsString();
#else
		::std::string str;
		switch (n->GetType()) {
			case Notification::Type_ValueAdded:
				str = "ValueAdded"; break;
			case Notification::Type_ValueRemoved:
				str = "ValueRemoved";		break;
			case Notification::Type_ValueChanged:
				str = "ValueChanged";		break;
			case Notification::Type_ValueRefreshed:
				str = "ValueRefreshed";	break;
			case Notification::Type_Group:
				str = "Group";	break;
			case Notification::Type_NodeNew:
				str = "NodeNew"; break;
			case Notification::Type_NodeAdded:
				str = "NodeAdded";	break;
			case Notification::Type_NodeRemoved:
				str = "NodeRemoved";		break;
			case Notification::Type_NodeProtocolInfo:
				str = "NodeProtocolInfo";	break;
			case Notification::Type_NodeNaming:
				str = "NodeNaming";			break;
			case Notification::Type_NodeEvent:
				str = "NodeEvent";			break;
			case Notification::Type_PollingDisabled:
				str = "PollingDisabled";break;
			case Notification::Type_PollingEnabled:
				str = "PollingEnabled";	break;
			case Notification::Type_SceneEvent:
				str = "SceneEvent";			break;
			case Notification::Type_CreateButton:
				str = "CreateButton";		break;
			case Notification::Type_DeleteButton:
				str = "DeleteButton";		break;
			case Notification::Type_ButtonOn:
				str = "ButtonOn";			break;
			case Notification::Type_ButtonOff:
				str = "ButtonOff";		break;
			case Notification::Type_DriverReady:
				str = "DriverReady";	break;
			case Notification::Type_DriverFailed:
				str = "DriverFailed";	break;
			case Notification::Type_DriverReset:
				str = "DriverReset";	break;
			case Notification::Type_EssentialNodeQueriesComplete:
				str = "EssentialNodeQueriesComplete";		break;
			case Notification::Type_NodeQueriesComplete:
				str = "NodeQueriesComplete";	break;
			case Notification::Type_AwakeNodesQueried:
				str = "AwakeNodesQueried";	break;
			case Notification::Type_AllNodesQueriedSomeDead:
				str = "AllNodesQueriedSomeDead";	break;
			case Notification::Type_AllNodesQueried:
				str = "AllNodesQueried";	break;
			case Notification::Type_Notification:
				str = "Notification - ";
				str.append(getControllerStateAsStr((OpenZWave::Driver::ControllerState) n->GetByte()));
				break;
			case Notification::Type_DriverRemoved:
				str = "DriverRemoved";	break;
			case Notification::Type_NodeReset:
				str = "NodeReset"; break;
			case Notification::Type_UserAlerts:
				str = "UserAlerts"; break;
			case Notification::Type_ManufacturerSpecificDBRead:
				str = "ManuacturerSpecificDBRead"; break;
		}
		return str;
#endif
}

}
