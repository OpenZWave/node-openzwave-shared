/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
* Copyright (c) 2015-2019 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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

    // ===================================================================
    NAN_METHOD(OZW::GetMetaData)
    // ===================================================================
    {   std::string result("");
        Nan::HandleScope scope;
        CheckMinArgs(2, "nodeid, metadata_name");
        uint8 _nodeId = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
        ::std::string metadata_name(*Nan::Utf8String( info[1] ));
        //
        OpenZWave::Node::MetaDataFields md_id;
        OZWManagerAssign ( md_id, GetMetaDataId, metadata_name );
        if (md_id != OpenZWave::Node::MetaData_Invalid) {
            OZWManagerAssign(result, GetMetaData, homeid, _nodeId, md_id);
        }
        info.GetReturnValue().Set(Nan::New<String>(result.c_str()).ToLocalChecked());
    }
    // ===================================================================
    NAN_METHOD(OZW::GetChangeLog)
    // ===================================================================
    {
        Nan::HandleScope scope;
        CheckMinArgs(2, "nodeid, revision");
        uint8 _nodeId  = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
        uint8 revision = Nan::To<Number>(info[1]).ToLocalChecked()->Value();
        OpenZWave::Node::ChangeLogEntry entry;
        OZWManagerAssign( entry, GetChangeLog, homeid, _nodeId, revision );
        Local<Object> o  = Nan::New<Object>();
        AddStringProp (o, author,      entry.author.c_str());
        AddStringProp (o, date,        entry.date.c_str());
        AddStringProp (o, description, entry.description.c_str());
        AddIntegerProp(o, revision,    entry.revision);
        info.GetReturnValue().Set(o);
    }


}