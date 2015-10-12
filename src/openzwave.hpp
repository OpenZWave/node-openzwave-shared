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
//=================================
#ifndef __OPENZWAVE_HPP_INCLUDED__
#define __OPENZWAVE_HPP_INCLUDED__

#include <iostream>
#include <list>
#include <queue>
#include <tr1/unordered_map>

#include <node.h>
#include <v8.h>
#include "nan.h"

#include "Manager.h"
#include "Driver.h"
#include "Node.h"
#include "Notification.h"
#include "Options.h"
#include "Value.h"

#define stringify( x ) stringify_literal( x )
#define stringify_literal( x ) # x

#ifdef WIN32
class mutex
{
public:
	mutex()              { InitializeCriticalSection(&_criticalSection); }
	~mutex()             { DeleteCriticalSection(&_criticalSection); }
	inline void lock()   { EnterCriticalSection(&_criticalSection); }
	inline void unlock() { LeaveCriticalSection(&_criticalSection); }

	class scoped_lock
	{
	public:
		inline explicit scoped_lock(mutex & sp) : _sl(sp) { _sl.lock(); }
		inline ~scoped_lock()                             { _sl.unlock(); }
	private:
		scoped_lock(scoped_lock const &);
		scoped_lock & operator=(scoped_lock const &);
		mutex& _sl;
	};

private:
	CRITICAL_SECTION _criticalSection;
};
#endif

#ifdef linux
#include <unistd.h>
#include <pthread.h>

class mutex
{
public:
	mutex()             { pthread_mutex_init(&_mutex, NULL); }
	~mutex()            { pthread_mutex_destroy(&_mutex); }
	inline void lock()  { pthread_mutex_lock(&_mutex); }
	inline void unlock(){ pthread_mutex_unlock(&_mutex); }

	class scoped_lock
	{
	public:
		inline explicit scoped_lock(mutex & sp) : _sl(sp)  { _sl.lock(); }
		inline ~scoped_lock()                              { _sl.unlock(); }
	private:
		scoped_lock(scoped_lock const &);
		scoped_lock & operator=(scoped_lock const &);
		mutex&  _sl;
	};

private:
	pthread_mutex_t _mutex;
};
#endif

using namespace v8;
using namespace node;

namespace OZW {

	struct OZW : public ObjectWrap {
		static NAN_METHOD(New);
		// openzwave-config.cc
		static NAN_METHOD(SetConfigParam);
		static NAN_METHOD(RequestConfigParam);
		static NAN_METHOD(RequestAllConfigParams);
		// openzwave-controller.cc
		static NAN_METHOD(HardReset);
		static NAN_METHOD(SoftReset);
		static NAN_METHOD(GetControllerNodeId);
		static NAN_METHOD(GetSUCNodeId);
		static NAN_METHOD(IsPrimaryController);
		static NAN_METHOD(IsStaticUpdateController);
		static NAN_METHOD(IsBridgeController);
		static NAN_METHOD(GetLibraryVersion);
		static NAN_METHOD(GetLibraryTypeName);
		static NAN_METHOD(GetSendQueueCount);
		// openzwave-driver.cc
		static NAN_METHOD(Connect);
		static NAN_METHOD(Disconnect);
		// openzwave-groups.cc
		static NAN_METHOD(GetNumGroups);
		static NAN_METHOD(GetAssociations);
		static NAN_METHOD(GetMaxAssociations);
		static NAN_METHOD(GetGroupLabel);
		static NAN_METHOD(AddAssociation);
		static NAN_METHOD(RemoveAssociation);
#if OPENZWAVE_SECURITY == 1
		static NAN_METHOD(AddNode);
		static NAN_METHOD(RemoveNode);
		static NAN_METHOD(RemoveFailedNode);
		static NAN_METHOD(HasNodeFailed);
		static NAN_METHOD(AssignReturnRoute);
		static NAN_METHOD(RequestNodeNeighborUpdate);
		static NAN_METHOD(DeleteAllReturnRoutes);
		static NAN_METHOD(SendNodeInformation);
		static NAN_METHOD(CreateNewPrimary);
		static NAN_METHOD(ReceiveConfiguration);
		static NAN_METHOD(ReplaceFailedNode);
		static NAN_METHOD(TransferPrimaryRole);
		static NAN_METHOD(RequestNetworkUpdate);
		static NAN_METHOD(ReplicationSend);
		static NAN_METHOD(CreateButton);
		static NAN_METHOD(DeleteButton);
#else
		static NAN_METHOD(BeginControllerCommand);
		static NAN_METHOD(CancelControllerCommand);
#endif
		// openzwave-network.cc
		static NAN_METHOD(TestNetworkNode);
		static NAN_METHOD(TestNetwork);
		static NAN_METHOD(HealNetworkNode);
		static NAN_METHOD(HealNetwork);
		// openzwave-nodes.cc
		static NAN_METHOD(SetNodeOn);
		static NAN_METHOD(SetNodeOff);
		static NAN_METHOD(SetNodeLevel);
		static NAN_METHOD(SwitchAllOn);
		static NAN_METHOD(SwitchAllOff);
		//
		static NAN_METHOD(RefreshNodeInfo);
		static NAN_METHOD(RequestNodeState);
		static NAN_METHOD(RequestNodeDynamic);
		static NAN_METHOD(IsNodeListeningDevice);
		static NAN_METHOD(IsNodeFrequentListeningDevice);
		static NAN_METHOD(IsNodeBeamingDevice);
		static NAN_METHOD(IsNodeRoutingDevice);
		static NAN_METHOD(IsNodeSecurityDevice);
		// getter+setter pairs
		static NAN_METHOD(GetNodeLocation);
		static NAN_METHOD(SetNodeLocation);
		static NAN_METHOD(GetNodeName);
		static NAN_METHOD(SetNodeName);
		static NAN_METHOD(GetNodeManufacturerName);
		static NAN_METHOD(SetNodeManufacturerName);
		static NAN_METHOD(GetNodeProductName);
		static NAN_METHOD(SetNodeProductName);
		// plain getters
		static NAN_METHOD(GetNodeMaxBaudRate);
		static NAN_METHOD(GetNodeVersion);
		static NAN_METHOD(GetNodeSecurity);
		static NAN_METHOD(GetNodeBasic);
		static NAN_METHOD(GetNodeGeneric);
		static NAN_METHOD(GetNodeSpecific);
		static NAN_METHOD(GetNodeType);
		static NAN_METHOD(GetNodeManufacturerId);
		static NAN_METHOD(GetNodeProductType);
		static NAN_METHOD(GetNodeProductId);
		static NAN_METHOD(GetNodeNeighbors);
		// openzwave-values.cc
		static NAN_METHOD(SetValue);
		// openzwave-polling.cc
		static NAN_METHOD(GetPollInterval);
		static NAN_METHOD(SetPollInterval);
		static NAN_METHOD(EnablePoll);
		static NAN_METHOD(DisablePoll);
		static NAN_METHOD(IsPolled);
		static NAN_METHOD(SetPollIntensity);
		static NAN_METHOD(GetPollIntensity);
		// openzwave-scenes.cc
		static NAN_METHOD(CreateScene);
		static NAN_METHOD(RemoveScene);
		static NAN_METHOD(GetScenes);
		static NAN_METHOD(AddSceneValue);
		static NAN_METHOD(RemoveSceneValue);
		static NAN_METHOD(SceneGetValues);
		static NAN_METHOD(ActivateScene);
	};

	// callback struct to copy data from the OZW thread to the v8 event loop:
	typedef struct {
		uint32 type;
		uint32 homeid;
		uint8 nodeid;
		uint8 groupidx;
		uint8 event;
		uint8 buttonid;
		uint8 sceneid;
		uint8 notification;
		std::list<OpenZWave::ValueID> values;
	} NotifInfo;

	typedef struct {
		uint32 homeid;
		uint8  nodeid;
		bool   polled;
		std::list<OpenZWave::ValueID> values;
	} NodeInfo;

	typedef struct {
		uint32      sceneid;
		std::string label;
		std::list<OpenZWave::ValueID> values;
	} SceneInfo;

	/*
	 */
	extern uv_async_t 		async;

	/*
	* Message passing queue between OpenZWave callback and v8 async handler.
	*/
	extern mutex zqueue_mutex;
	extern std::queue<NotifInfo *> zqueue;

	/*
	* Node state.
	*/
	extern mutex znodes_mutex;
	extern std::list<NodeInfo *> znodes;

	extern mutex zscenes_mutex;
	extern std::list<SceneInfo *> zscenes;

	// our ZWave Home ID
	extern uint32 homeid;

	Local<Object> zwaveValue2v8Value(OpenZWave::ValueID value);
	Local<Object> zwaveSceneValue2v8Value(uint8 sceneId, OpenZWave::ValueID value);

	NodeInfo  *get_node_info(uint8 nodeid);
	SceneInfo *get_scene_info(uint8 sceneid);

	// OpenZWave callbacks
	void ozw_watcher_callback(
		OpenZWave::Notification const *cb,
		void *ctx);
	void ozw_ctrlcmd_callback(
		OpenZWave::Driver::ControllerState _state,
		OpenZWave::Driver::ControllerError _err,
		void *ctx);

	// v8 asynchronous callback handler
	void async_cb_handler(uv_async_t *handle);
	void async_cb_handler(uv_async_t *handle, int status);

	//extern Handle<Object>	context_obj;
	extern Nan::Callback *emit_cb;
	//

	// map of controller command names to enum values
	typedef ::std::tr1::unordered_map <std::string, OpenZWave::Driver::ControllerCommand> CommandMap;
	extern CommandMap* ctrlCmdNames;

}

#endif // __OPENZWAVE_HPP_INCLUDED__
