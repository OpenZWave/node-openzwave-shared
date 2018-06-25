/*
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
//=================================
#ifndef __NODE_OPENZWAVE_CALLBACKS_HPP_INCLUDED__
#define __NODE_OPENZWAVE_CALLBACKS_HPP_INCLUDED__

namespace OZW {

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
    std::string help;
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

  extern Nan::Callback *emit_cb;
  extern Nan::CopyablePersistentTraits<v8::Object>::CopyablePersistent ctx_obj;
  extern Nan::AsyncResource *resource;

  /*
  * uv_async to let the OpenZWave callback wake up the main V8 thread
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
  extern std::map<uint8_t, NodeInfo *> znodes;

  extern mutex zscenes_mutex;
  extern std::list<SceneInfo *> zscenes;

} // namespace OZW

#endif // __NODE_OPENZWAVE_CALLBACKS_HPP_INCLUDED__
