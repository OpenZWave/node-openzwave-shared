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

#include "Manager.h"
#include "Driver.h"
#include "Node.h"
#include "Notification.h"
#include "Options.h"
#include "Value.h"

#define stringify( name ) # name

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
	
	struct OZW : ObjectWrap {
		static Handle<v8::Value> New(const Arguments& args);
		// openzwave-config.cc
		static Handle<v8::Value> SetConfigParam(const Arguments& args);
		static Handle<v8::Value> RequestConfigParam(const Arguments& args);
		static Handle<v8::Value> RequestAllConfigParams(const Arguments& args);
		// openzwave-controller.cc
		static Handle<v8::Value> HardReset(const Arguments& args);
		static Handle<v8::Value> SoftReset(const Arguments& args);
		static Handle<v8::Value> BeginControllerCommand(const Arguments& args);
		static Handle<v8::Value> CancelControllerCommand(const Arguments& args);
		// openzwave-driver.cc
		static Handle<v8::Value> Connect(const Arguments& args);
		static Handle<v8::Value> Disconnect(const Arguments& args);
		// openzwave-groups.cc
		static Handle<v8::Value> GetNumGroups(const Arguments& args);
		static Handle<v8::Value> GetAssociations(const Arguments& args);
		static Handle<v8::Value> GetMaxAssociations(const Arguments& args);
		static Handle<v8::Value> GetGroupLabel(const Arguments& args);
		static Handle<v8::Value> AddAssociation(const Arguments& args);
		static Handle<v8::Value> RemoveAssociation(const Arguments& args);
		// openzwave-network.cc
		static Handle<v8::Value> TestNetworkNode(const Arguments& args);
		static Handle<v8::Value> TestNetwork(const Arguments& args);
		static Handle<v8::Value> HealNetworkNode(const Arguments& args);
		static Handle<v8::Value> HealNetwork(const Arguments& args);
		// openzwave-nodes.cc
		static Handle<v8::Value> GetNodeNeighbors(const Arguments& args);
		static Handle<v8::Value> RefreshNodeInfo(const Arguments& args);
		static Handle<v8::Value> SwitchAllOn(const Arguments& args);
		static Handle<v8::Value> SwitchAllOff(const Arguments& args);
		// openzwave-values.cc
		static Handle<v8::Value> SetValue(const Arguments& args);
		static Handle<v8::Value> SetLocation(const Arguments& args);
		static Handle<v8::Value> SetName(const Arguments& args);
		// openzwave-polling.cc
		static Handle<v8::Value> EnablePoll(const Arguments& args);
		static Handle<v8::Value> DisablePoll(const Arguments& args);
		static Handle<v8::Value> SetPollInterval(const Arguments& args);
		static Handle<v8::Value> SetPollIntensity(const Arguments& args);
		// openzwave-scenes.cc
		static Handle<v8::Value> CreateScene(const Arguments& args);
		static Handle<v8::Value> RemoveScene(const Arguments& args);
		static Handle<v8::Value> GetScenes(const Arguments& args);
		static Handle<v8::Value> AddSceneValue(const Arguments& args);
		static Handle<v8::Value> RemoveSceneValue(const Arguments& args);
		static Handle<v8::Value> SceneGetValues(const Arguments& args);
		static Handle<v8::Value> ActivateScene(const Arguments& args);
		//
		
		//
	};

	// callback struct to copy data from the OZW thread to the v8 event loop: 
	typedef struct {
		uint32_t type;
		uint32_t homeid;
		uint8_t nodeid;
		uint8_t groupidx;
		uint8_t event;
		uint8_t buttonid;
		uint8_t sceneid;
		uint8_t notification;
		std::list<OpenZWave::ValueID> values;
		OpenZWave::Driver::ControllerState state;
		OpenZWave::Driver::ControllerError err;
	} NotifInfo;
	
	typedef struct {
		uint32_t homeid;
		uint8_t nodeid;
		bool polled;
		std::list<OpenZWave::ValueID> values;
	} NodeInfo;

	typedef struct {
		uint32_t sceneid;
		std::string label;
		std::list<OpenZWave::ValueID> values;
	} SceneInfo;
	
	/*  
	 */
	extern uv_async_t 		async;
	extern Persistent<Object> context_obj;
	
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
	extern uint32_t homeid;

	Local<Object> zwaveValue2v8Value(OpenZWave::ValueID value);
	Local<Object> zwaveSceneValue2v8Value(uint8 sceneId, OpenZWave::ValueID value);
	
	NodeInfo *get_node_info(uint8_t nodeid);
	SceneInfo *get_scene_info(uint8_t sceneid);
	
	// OpenZWave callbacks
	void ozw_watcher_callback(
		OpenZWave::Notification const *cb, 
		void *ctx);
	void ozw_ctrlcmd_callback(
		OpenZWave::Driver::ControllerState _state, 
		OpenZWave::Driver::ControllerError _err, 
		void *ctx);
		
	// v8 asynchronous callback handler
	void async_cb_handler(uv_async_t *handle, int status);
	//
	
	// map of controller command names to enum values
	typedef ::std::tr1::unordered_map <std::string, OpenZWave::Driver::ControllerCommand> CommandMap;
	extern CommandMap* ctrlCmdNames;
	
}

#endif // __OPENZWAVE_HPP_INCLUDED__
