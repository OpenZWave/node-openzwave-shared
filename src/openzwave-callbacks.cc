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
using namespace OpenZWave;

namespace OZW {
	/*
	* OpenZWave callback, registered in Driver::AddWatcher.
	* Just push onto queue and trigger the handler
	* in v8 land.
	*/
	// ===================================================================
	void ozw_watcher_callback(OpenZWave::Notification const *cb, void *ctx)
	// ===================================================================
	{
		NotifInfo *notif = new NotifInfo();
		
		notif->type = cb->GetType();
		notif->homeid = cb->GetHomeId();
		notif->nodeid = cb->GetNodeId();
		notif->values.push_front(cb->GetValueID());
		// its not a ControllerState notification: set to -1
		notif->state = (OpenZWave::Driver::ControllerState) -1;
		/*
		* Some values are only set on particular notifications, and
		* assertions in openzwave prevent us from trying to fetch them
		* unconditionally.
		*/
		switch (notif->type) {
			case OpenZWave::Notification::Type_Group:
				notif->groupidx = cb->GetGroupIdx();
				break;
			case OpenZWave::Notification::Type_NodeEvent:
				notif->event = cb->GetEvent();
				break;
			case OpenZWave::Notification::Type_CreateButton:
			case OpenZWave::Notification::Type_DeleteButton:
			case OpenZWave::Notification::Type_ButtonOn:
			case OpenZWave::Notification::Type_ButtonOff:
				notif->buttonid = cb->GetButtonId();
				break;
			case OpenZWave::Notification::Type_SceneEvent:
				notif->sceneid = cb->GetSceneId();
				break;
			case OpenZWave::Notification::Type_Notification:
				notif->notification = cb->GetNotification();
				break;
		}

		{
			mutex::scoped_lock sl(zqueue_mutex);
			zqueue.push(notif);
		}
		uv_async_send(&async);
	}
	
	/*
	* OpenZWave callback, registered in Manager::BeginControllerCommand.
	* Just push onto queue and trigger the handler in v8 land.
	*/
	// ===================================================================
	void ozw_ctrlcmd_callback(Driver::ControllerState _state, Driver::ControllerError _err, void *context )
	// ===================================================================
	{
		NotifInfo *notif = new NotifInfo();
		notif->state = _state;
		notif->err   = _err;
		{
			mutex::scoped_lock sl(zqueue_mutex);
			zqueue.push(notif);
		}
		uv_async_send(&async);
	}
}
