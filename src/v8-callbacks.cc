/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
* Copyright (c) 2015 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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

using namespace v8;
using namespace node;

namespace OZW {
	/*
	 * handle normal OpenZWave notifications
	 */ 
	// ===================================================================
	void handleNotification(NotifInfo *notif) 
	// ===================================================================
	{
		NodeInfo *node;
		
		Local < v8::Value > args[16];
		switch (notif->type) {
			case OpenZWave::Notification::Type_DriverReady:
				homeid = notif->homeid;
				args[0] = NanNew<String>("driver ready");
				args[1] = NanNew<Integer>(homeid);
				MakeCallback(context_obj, "emit", 2, args);
				break;
			case OpenZWave::Notification::Type_DriverFailed:
				args[0] = NanNew<String>("driver failed");
				MakeCallback(context_obj, "emit", 1, args);
				break;
				/*
				* NodeNew is triggered when a node is discovered which is not
				* found in the OpenZWave XML file.  As we do not use that file
				* simply ignore those notifications for now.
				*
				* NodeAdded is when we actually have a new node to set up.
				*/
			case OpenZWave::Notification::Type_NodeNew:
				break;
			case OpenZWave::Notification::Type_NodeAdded:
				node = new NodeInfo();
				node->homeid = notif->homeid;
				node->nodeid = notif->nodeid;
				node->polled = false;
				{
					mutex::scoped_lock sl(znodes_mutex);
					znodes.push_back(node);
				}
				args[0] = NanNew<String>("node added");
				args[1] = NanNew<Integer>(notif->nodeid);
				MakeCallback(context_obj, "emit", 2, args);
				break;
				/*
				* Ignore intermediate notifications about a node status, we
				* wait until the node is ready before retrieving information.
				*/
			case OpenZWave::Notification::Type_NodeProtocolInfo:
				break;
			case OpenZWave::Notification::Type_NodeNaming: {
				Local < Object > info = Object::New();
				info->Set(NanNew<String>("manufacturer"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeManufacturerName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("manufacturerid"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeManufacturerId(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("product"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("producttype"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductType(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("productid"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductId(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("type"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeType(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("name"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("loc"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeLocation(notif->homeid, notif->nodeid).c_str()));
				args[0] = NanNew<String>("node naming");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = info;
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
			// TODO: these should be supported correctly.
			case OpenZWave::Notification::Type_PollingEnabled:
			case OpenZWave::Notification::Type_PollingDisabled:
				break;
				/*
				* Node values.
				*/
			case OpenZWave::Notification::Type_ValueAdded: {
				OpenZWave::ValueID value = notif->values.front();
				Local<Object> valobj = zwaveValue2v8Value(value);

				if ((node = get_node_info(notif->nodeid))) {
					mutex::scoped_lock sl(znodes_mutex);
					node->values.push_back(value);
				}

				args[0] = NanNew<String>("value added");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = NanNew<Integer>(value.GetCommandClassId());
				args[3] = valobj;
				MakeCallback(context_obj, "emit", 4, args);
				break;
			}
			case OpenZWave::Notification::Type_ValueChanged: {
				OpenZWave::ValueID value = notif->values.front();
				Local<Object> valobj = zwaveValue2v8Value(value);

				args[0] = NanNew<String>("value changed");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = NanNew<Integer>(value.GetCommandClassId());
				args[3] = valobj;
				MakeCallback(context_obj, "emit", 4, args);
				break;
			}
			case OpenZWave::Notification::Type_ValueRefreshed: {
				OpenZWave::ValueID value = notif->values.front();
				Local<Object> valobj = zwaveValue2v8Value(value);

				args[0] = NanNew<String>("value refreshed");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = NanNew<Integer>(value.GetCommandClassId());
				args[3] = valobj;
				MakeCallback(context_obj, "emit", 4, args);
				break;
			}
			case OpenZWave::Notification::Type_ValueRemoved: {
				OpenZWave::ValueID value = notif->values.front();
				std::list<OpenZWave::ValueID>::iterator vit;
				if ((node = get_node_info(notif->nodeid))) {
					for (vit = node->values.begin(); vit != node->values.end(); ++vit) {
						if ((*vit) == notif->values.front()) {
							node->values.erase(vit);
							break;
						}
					}
				}
				args[0] = NanNew<String>("value removed");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = NanNew<Integer>(value.GetCommandClassId());
				args[3] = NanNew<Integer>(value.GetInstance());
				args[4] = NanNew<Integer>(value.GetIndex());
				MakeCallback(context_obj, "emit", 5, args);
				break;
			}
			/*
			 *Now node can accept commands.
			 */
			case OpenZWave::Notification::Type_EssentialNodeQueriesComplete: {
				Local < Object > info = Object::New();
				info->Set(NanNew<String>("manufacturer"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeManufacturerName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("manufacturerid"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeManufacturerId(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("product"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("producttype"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductType(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("productid"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductId(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("type"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeType(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("name"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("loc"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeLocation(notif->homeid, notif->nodeid).c_str()));
				args[0] = NanNew<String>("node available");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = info;
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
			/*
			* The node is now fully ready for operation.
			*/
			case OpenZWave::Notification::Type_NodeQueriesComplete: {
				Local < Object > info = Object::New();
				info->Set(NanNew<String>("manufacturer"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeManufacturerName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("manufacturerid"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeManufacturerId(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("product"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("producttype"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductType(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("productid"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeProductId(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("type"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeType(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("name"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeName(notif->homeid, notif->nodeid).c_str()));
				info->Set(NanNew<String>("loc"), NanNew<String>(OpenZWave::Manager::Get()->GetNodeLocation(notif->homeid, notif->nodeid).c_str()));
				args[0] = NanNew<String>("node ready");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = info;
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
			/*
			* The network scan has been completed.  Currently we do not
			* care about dead nodes - is there anything we can do anyway?
			*/
			case OpenZWave::Notification::Type_AwakeNodesQueried:
			case OpenZWave::Notification::Type_AllNodesQueried:
			case OpenZWave::Notification::Type_AllNodesQueriedSomeDead:
				args[0] = NanNew<String>("scan complete");
				MakeCallback(context_obj, "emit", 1, args);
				break;
			case OpenZWave::Notification::Type_NodeEvent: {
				args[0] = NanNew<String>("node event");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = NanNew<Integer>(notif->event);
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
			case OpenZWave::Notification::Type_SceneEvent:{
				args[0] = NanNew<String>("scene event");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = NanNew<Integer>(notif->sceneid);
				MakeCallback(context_obj, "emit", 3, args);
				break;
			}
			case OpenZWave::Notification::Type_Notification:
				args[0] = NanNew<String>("notification");
				args[1] = NanNew<Integer>(notif->nodeid);
				args[2] = NanNew<Integer>(notif->notification);
				MakeCallback(context_obj, "emit", 3, args);
				break;
			case OpenZWave::Notification::Type_Group:
				// TODO:
				/* The associations for the node have changed. The 
				 * application should rebuild any group information it 
				 * holds about the node.
				 */
				break;
				/*
				* Send unhandled events to stderr so we can monitor them if
				* necessary.
				*/
			default:
				fprintf(stderr, "Unhandled notification: %d\n", notif->type);
				break;
		} // end switch
	}
	
	// ===================================================================
	void handleControllerCommand(NotifInfo *notif) 
	// ===================================================================
	{
		Local < v8::Value > args[16];
		args[0] = NanNew<String>("controller command");
		args[1] = NanNew<Integer>(notif->state);
		args[2] = NanNew<Integer>(notif->err);
		MakeCallback(context_obj, "emit", 3, args);
	}
	
	/*
	* Async handler, triggered by the OpenZWave callback.
	*/
	// ===================================================================
	void async_cb_handler(uv_async_t *handle, int status)
	// ===================================================================
	{
		NotifInfo *notif;

		mutex::scoped_lock sl(zqueue_mutex);

		while (!zqueue.empty()) {
			notif = zqueue.front();
			if (notif->state < 0) {
				handleNotification(notif);
			} else {
				handleControllerCommand(notif);
			}
			zqueue.pop();
		}
	}
}
