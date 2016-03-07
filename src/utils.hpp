/*
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
#ifndef __NODE_OPENZWAVE_UTILS_HPP_INCLUDED__
#define __NODE_OPENZWAVE_UTILS_HPP_INCLUDED__

#define stringify( x ) stringify_literal( x )
#define stringify_literal( x ) # x

namespace OZW {

#ifdef WIN32
	class mutex {
	private:
		CRITICAL_SECTION     _criticalSection;
	public:
		mutex()              { InitializeCriticalSection(&_criticalSection); }
		~mutex()             { DeleteCriticalSection(&_criticalSection); }
		inline void lock()   { EnterCriticalSection(&_criticalSection); }
		inline void unlock() { LeaveCriticalSection(&_criticalSection); }

#else // Linux, MacOS
#include <unistd.h>
#include <pthread.h>
	class mutex {
	private:
		pthread_mutex_t _mutex;

	public:
		mutex()             { pthread_mutex_init(&_mutex, NULL); }
		~mutex()            { pthread_mutex_destroy(&_mutex); }
		inline void lock()  { pthread_mutex_lock(&_mutex); }
		inline void unlock(){ pthread_mutex_unlock(&_mutex); }
#endif

	class scoped_lock {
		public:
			inline explicit scoped_lock(mutex & sp) : _sl(sp) { _sl.lock(); }
			inline ~scoped_lock()                             { _sl.unlock(); }
		private:
			scoped_lock(scoped_lock const &);
			scoped_lock & operator=(scoped_lock const &);
			mutex& _sl;
		};

	};

};

#include "callbacks.hpp"

namespace OZW {

	v8::Local<v8::Object> zwaveValue2v8Value(OpenZWave::ValueID value);
	v8::Local<v8::Object> zwaveSceneValue2v8Value(uint8 sceneId, OpenZWave::ValueID value);

	NodeInfo  *get_node_info(uint8 nodeid);
	SceneInfo *get_scene_info(uint8 sceneid);

	OpenZWave::ValueID* getZwaveValueID(const Nan::FunctionCallbackInfo<v8::Value>& info, uint8 offset=0);
	const char* getControllerStateAsStr (OpenZWave::Driver::ControllerState _state);
	const char* getControllerErrorAsStr(OpenZWave::Driver::ControllerError _err);

	const std::string getNotifHelpMsg(OpenZWave::Notification const *n);
	void getV8ValueForZWaveNode(
			OpenZWave::Manager *mgr,
			v8::Local<v8::Object>& nodeobj,
			uint32 homeid, uint8 nodeid
	);

	bool checkType(bool predicate);
} // namespace OZW

#endif // __NODE_OPENZWAVE_UTILS_HPP_INCLUDED__
