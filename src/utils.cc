/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
* Copyright (c) 2013 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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

	SceneInfo *get_scene_info(uint8 sceneid) {
		std::list<SceneInfo *>::iterator it;

		SceneInfo *scene;

		for (it = zscenes.begin(); it != zscenes.end(); ++it) {
			scene = *it;
			if (scene->sceneid == sceneid)
				return scene;
		}

		return NULL;
	}

	// populate a v8 object with an attribute called 'value' whose value is the
	// ZWave value, as returned from its proper typed call
	void setValObj(Local<Object>&valobj, OpenZWave::ValueID &value) {
		/*
		* The value itself is type-specific.
		*/
		switch (value.GetType()) {
			case OpenZWave::ValueID::ValueType_Bool: {
				bool val;
				OpenZWave::Manager::Get()->GetValueAsBool(value, &val);
				Nan::Set(valobj,
					Nan::New<String>("value").ToLocalChecked(),
					Nan::New<Boolean>(val)->ToBoolean()
				);
				break;
			}
			case OpenZWave::ValueID::ValueType_Byte: {
				uint8 val;
				OpenZWave::Manager::Get()->GetValueAsByte(value, &val);
				Nan::Set(valobj,
					Nan::New<String>("value").ToLocalChecked(),
					Nan::New<Integer>(val)
				);
				break;
			}
			case OpenZWave::ValueID::ValueType_Decimal: {
				std::string val;
				OpenZWave::Manager::Get()->GetValueAsString(value, &val);
				Nan::Set(valobj,
					Nan::New<String>("value").ToLocalChecked(),
					Nan::New<String>(val).ToLocalChecked()
				);
				break;
			}
			case OpenZWave::ValueID::ValueType_Int: {
				int32 val;
				OpenZWave::Manager::Get()->GetValueAsInt(value, &val);
				Nan::Set(valobj,
					Nan::New<String>("value").ToLocalChecked(),
					Nan::New<Integer>(val)
				);
				break;
			}
			case OpenZWave::ValueID::ValueType_List: {
				std::vector < std::string > items;
				OpenZWave::Manager::Get()->GetValueListItems(value, &items);
				Local < Array > values = Nan::New<Array>(items.size());
				for (unsigned int i = 0; i < items.size(); i++) {
					Nan::Set(values, i,
						Nan::New<String>(&items[i][0], items[i].size()).ToLocalChecked()
					);
				}
				Nan::Set(valobj,
					Nan::New<String>("values").ToLocalChecked(),
					values
				);
				std::string val;
				OpenZWave::Manager::Get()->GetValueListSelection(value, &val);
				Nan::Set(valobj,
					Nan::New<String>("value").ToLocalChecked(),
					Nan::New<String>(val.c_str()).ToLocalChecked()
				);
				break;
			}
			case OpenZWave::ValueID::ValueType_Short: {
				int16 val;
				OpenZWave::Manager::Get()->GetValueAsShort(value, &val);
				Nan::Set(valobj,
					Nan::New<String>("value").ToLocalChecked(),
					Nan::New<Integer>(val)
				);
				break;
			}
			case OpenZWave::ValueID::ValueType_String: {
				std::string val;
				OpenZWave::Manager::Get()->GetValueAsString(value, &val);
				Nan::Set(valobj,
					Nan::New<String>("value").ToLocalChecked(),
					Nan::New<String>(val.c_str()).ToLocalChecked()
				);
				break;
			}
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
				OpenZWave::Manager::Get()->GetValueAsRaw(value, &val, &len);
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

	// populate a v8 Object with useful information about a ZWave node
	void getV8ValueForZWaveNode(
			OpenZWave::Manager *mgr,
			v8::Local<v8::Object>& nodeobj,
			uint32 homeid, uint8 nodeid
	) {
		Nan::Set(nodeobj,
			Nan::New<String>("manufacturer").ToLocalChecked(),
			Nan::New<String>(mgr->GetNodeManufacturerName(homeid, nodeid).c_str()).ToLocalChecked());
		Nan::Set(nodeobj,
			Nan::New<String>("manufacturerid").ToLocalChecked(),
			Nan::New<String>(mgr->GetNodeManufacturerId(homeid, nodeid).c_str()).ToLocalChecked());
		Nan::Set(nodeobj,
			Nan::New<String>("product").ToLocalChecked(),
			Nan::New<String>(mgr->GetNodeProductName(homeid, nodeid).c_str()).ToLocalChecked());
		Nan::Set(nodeobj,
			Nan::New<String>("producttype").ToLocalChecked(),
			Nan::New<String>(mgr->GetNodeProductType(homeid, nodeid).c_str()).ToLocalChecked());
		Nan::Set(nodeobj,
			Nan::New<String>("productid").ToLocalChecked(),
			Nan::New<String>(mgr->GetNodeProductId(homeid, nodeid).c_str()).ToLocalChecked());
		Nan::Set(nodeobj,
			Nan::New<String>("type").ToLocalChecked(),
			Nan::New<String>(mgr->GetNodeType(homeid, nodeid).c_str()).ToLocalChecked());
		Nan::Set(nodeobj,
			Nan::New<String>("name").ToLocalChecked(),
			Nan::New<String>(mgr->GetNodeName(homeid, nodeid).c_str()).ToLocalChecked());
		Nan::Set(nodeobj,
			Nan::New<String>("loc").ToLocalChecked(),
			Nan::New<String>(mgr->GetNodeLocation(homeid, nodeid).c_str()).ToLocalChecked());
	}

	// create a V8 object from a ZWave value
	Local<Object> zwaveValue2v8Value(OpenZWave::ValueID value) {
		Nan::EscapableHandleScope handle_scope;

		char buffer[15];
		sprintf(buffer, "%d-%d-%d-%d", value.GetNodeId(), value.GetCommandClassId(), value.GetInstance(), value.GetIndex());

		Local <Object> valobj = Nan::New<Object>();
		Nan::Set(valobj,
			Nan::New<String>("value_id").ToLocalChecked(),
			Nan::New<String>(buffer).ToLocalChecked());

		OpenZWave::Manager *mgr = OpenZWave::Manager::Get();
		/*
		* Common value types.
		*/
		// no 64-bit ints in Javascript:
		//Nan::Set(valobj, Nan::New<String>("id"), Nan::New<Integer>(value.GetId()));
		Nan::Set(valobj, Nan::New<String>("node_id").ToLocalChecked(),   Nan::New<Integer>(value.GetNodeId()));
		Nan::Set(valobj, Nan::New<String>("class_id").ToLocalChecked(),  Nan::New<Integer>(value.GetCommandClassId()));
		Nan::Set(valobj, Nan::New<String>("type").ToLocalChecked(),
			Nan::New<String> (
				OpenZWave::Value::GetTypeNameFromEnum(value.GetType())).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("genre").ToLocalChecked(),
			Nan::New<String> (
				OpenZWave::Value::GetGenreNameFromEnum(value.GetGenre())).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("instance").ToLocalChecked(),  Nan::New<Integer>(value.GetInstance()));
		Nan::Set(valobj, Nan::New<String>("index").ToLocalChecked(),     Nan::New<Integer>(value.GetIndex()));
		Nan::Set(valobj, Nan::New<String>("label").ToLocalChecked(),     Nan::New<String> (mgr->GetValueLabel(value).c_str()).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("units").ToLocalChecked(),     Nan::New<String> (mgr->GetValueUnits(value).c_str()).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("help").ToLocalChecked(),      Nan::New<String> (mgr->GetValueHelp(value).c_str()).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("read_only").ToLocalChecked(), Nan::New<Boolean>(mgr->IsValueReadOnly(value))->ToBoolean());
		Nan::Set(valobj, Nan::New<String>("write_only").ToLocalChecked(),Nan::New<Boolean>(mgr->IsValueWriteOnly(value))->ToBoolean());
		Nan::Set(valobj, Nan::New<String>("is_polled").ToLocalChecked(), Nan::New<Boolean>(mgr->IsValuePolled(value))->ToBoolean());
		//Nan::Set(valobj, Nan::New<String>("change_verified").ToLocalChecked(), Nan::New<Boolean>(mgr->GetChangeVerified(value))->ToBoolean());
		Nan::Set(valobj, Nan::New<String>("min").ToLocalChecked(), Nan::New<Integer>(OpenZWave::Manager::Get()->GetValueMin(value)));
		Nan::Set(valobj, Nan::New<String>("max").ToLocalChecked(), Nan::New<Integer>(OpenZWave::Manager::Get()->GetValueMax(value)));
		setValObj(valobj, value);

  	return handle_scope.Escape(valobj);
	}

	// create a V8 object from a ZWave scene value
	Local<Object> zwaveSceneValue2v8Value(uint8 sceneId, OpenZWave::ValueID value) {
		Nan::EscapableHandleScope handle_scope;

		char buffer[15];
		sprintf(buffer, "%d-%d-%d-%d", value.GetNodeId(), value.GetCommandClassId(), value.GetInstance(), value.GetIndex());

		Local <Object> valobj = Nan::New<Object>();
		Nan::Set(valobj,
			Nan::New<String>("value_id").ToLocalChecked(),
			Nan::New<String>(buffer).ToLocalChecked()
		);

		/*
		* Common value types.
		*/
		// Nan::Set(valobj, Nan::New<String>("id"), Nan::New<Integer>(value.GetId()));
		Nan::Set(valobj, Nan::New<String>("node_id").ToLocalChecked(),    Nan::New<Integer>(value.GetNodeId()));
		Nan::Set(valobj, Nan::New<String>("class_id").ToLocalChecked(),   Nan::New<Integer>(value.GetCommandClassId()));
		Nan::Set(valobj, Nan::New<String>("type").ToLocalChecked(),       Nan::New<String>(OpenZWave::Value::GetTypeNameFromEnum(value.GetType())).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("genre").ToLocalChecked(),      Nan::New<String>(OpenZWave::Value::GetGenreNameFromEnum(value.GetGenre())).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("instance").ToLocalChecked(),   Nan::New<Integer>(value.GetInstance()));
		Nan::Set(valobj, Nan::New<String>("index").ToLocalChecked(),      Nan::New<Integer>(value.GetIndex()));
		Nan::Set(valobj, Nan::New<String>("label").ToLocalChecked(),      Nan::New<String>(OpenZWave::Manager::Get()->GetValueLabel(value).c_str()).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("units").ToLocalChecked(),      Nan::New<String>(OpenZWave::Manager::Get()->GetValueUnits(value).c_str()).ToLocalChecked());
		Nan::Set(valobj, Nan::New<String>("read_only").ToLocalChecked(),  Nan::New<Boolean>(OpenZWave::Manager::Get()->IsValueReadOnly(value))->ToBoolean());
		Nan::Set(valobj, Nan::New<String>("write_only").ToLocalChecked(), Nan::New<Boolean>(OpenZWave::Manager::Get()->IsValueWriteOnly(value))->ToBoolean());
		// XXX: verify_changes=
		// XXX: poll_intensity=
		Nan::Set(valobj, Nan::New<String>("min").ToLocalChecked(), Nan::New<Integer>(OpenZWave::Manager::Get()->GetValueMin(value)));
		Nan::Set(valobj, Nan::New<String>("max").ToLocalChecked(), Nan::New<Integer>(OpenZWave::Manager::Get()->GetValueMax(value)));
		setValObj(valobj, value);
		return handle_scope.Escape(valobj);
	}

  bool isOzwValue(Local<Object>& o) {
		return (!Nan::HasOwnProperty(o, Nan::New<String>("node_id").ToLocalChecked()).IsNothing()
			&& !Nan::HasOwnProperty(o, Nan::New<String>("class_id").ToLocalChecked()).IsNothing()
			&& !Nan::HasOwnProperty(o, Nan::New<String>("instance").ToLocalChecked()).IsNothing()
			&& !Nan::HasOwnProperty(o, Nan::New<String>("index").ToLocalChecked()).IsNothing());
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
	OpenZWave::ValueID* getZwaveValueID(const Nan::FunctionCallbackInfo<v8::Value> &info, uint8 offset) {
		uint8 nodeid, comclass, instance, index;
		if ( (info.Length() >= offset) && info[offset]->IsObject() ) {
			Local<Object> o = info[offset]->ToObject();
			if (isOzwValue(o)) {
				nodeid   = Nan::Get(o, Nan::New<String>("node_id").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value();
				comclass = Nan::Get(o, Nan::New<String>("class_id").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value();
				instance = Nan::Get(o, Nan::New<String>("instance").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value();
				index    = Nan::Get(o, Nan::New<String>("index").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value();
			} else {
				return ( NULL );
			}
		} else if ((info.Length() >= offset+4)) {
			// legacy mode
			nodeid   = info[offset]->ToNumber()->Value();
			comclass = info[offset+1]->ToNumber()->Value();
			instance = info[offset+2]->ToNumber()->Value();
			index    = info[offset+3]->ToNumber()->Value();
		} else {
			return( NULL );
		}

		NodeInfo *node = NULL;
		std::list<OpenZWave::ValueID>::iterator vit;

		if ((node = get_node_info(nodeid))) {
			for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
				if (((*vit).GetCommandClassId() == comclass) && ((*vit).GetInstance() == instance) && ((*vit).GetIndex() == index)) {
					return ( &*vit );
				}
			}
		}
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
const std::string getNotifHelpMsg(Notification const *n) {
#if OPENZWAVE_SECURITY == 1
		return n->GetAsString();
#else
		std::string str;
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
				str = "DriverRemoved";				break;
		}
		return str;
#endif
}

}
