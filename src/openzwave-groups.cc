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
using namespace v8;
using namespace node;

namespace OZW {
	/*
	 * Gets the number of association groups reported by this node.
	 * In Z-Wave, groups are numbered starting from one. For example,
	 * if a call to GetNumGroups returns 4, the _groupIdx value to use
	 * in calls to GetAssociations, AddAssociation and RemoveAssociation
	 * will be a number between 1 and 4.
	*/
	// ===================================================================
	NAN_METHOD(OZW::GetNumGroups)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "nodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 numGroups = 0;
		OZWManagerAssign(numGroups, GetNumGroups, homeid, nodeid);
		info.GetReturnValue().Set(Nan::New<Integer>(numGroups));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetAssociations)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, groupidx");
		uint8* associations;
		uint8 nodeid   = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 groupidx = Nan::To<Number>(info[1]).ToLocalChecked()->Value();

		uint32 numNodes = 0;
		OZWManagerAssign(numNodes, GetAssociations,
			homeid, nodeid,	groupidx, &associations
		);

		Local<Array> o_assocs = Nan::New<Array>(numNodes);

		for (uint8 nr = 0; nr < numNodes; nr++) {
			Nan::Set(o_assocs, nr, Nan::New<Integer>(associations[nr]));
		}
		if (numNodes > 0) {
			// The caller is responsible for freeing the array memory with a call to delete [].
			delete associations;
		}

		info.GetReturnValue().Set(o_assocs);
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetMaxAssociations)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, groupidx");
		uint8 nodeid   = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 groupidx = Nan::To<Number>(info[1]).ToLocalChecked()->Value();

		uint8 numMaxAssoc = 0;
		OZWManagerAssign(numMaxAssoc, GetMaxAssociations,
			homeid, nodeid,	groupidx
		);

		info.GetReturnValue().Set(Nan::New<Integer>(numMaxAssoc));
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::GetGroupLabel)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "nodeid, groupidx");
		uint8 nodeid   = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 groupidx = Nan::To<Number>(info[1]).ToLocalChecked()->Value();

		::std::string groupLabel("");
		OZWManagerAssign(groupLabel, GetGroupLabel,
			homeid, nodeid, groupidx
		);

		info.GetReturnValue().Set(
			Nan::New<String>(groupLabel.c_str())
							.ToLocalChecked()
		);
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::AddAssociation)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(3, "nodeid, groupidx, tgtnodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 groupidx = Nan::To<Number>(info[1]).ToLocalChecked()->Value();
		uint8 tgtnodeid = Nan::To<Number>(info[2]).ToLocalChecked()->Value();
		uint8 instanceid = 0;
		if(info.Length() > 3) {
			instanceid = Nan::To<Number>(info[3]).ToLocalChecked()->Value();
		}

		OZWManager( AddAssociation,
			homeid, nodeid, groupidx, tgtnodeid, instanceid
		);
	}

	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::RemoveAssociation)
	// ===================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(3, "nodeid, groupidx, tgtnodeid");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 groupidx = Nan::To<Number>(info[1]).ToLocalChecked()->Value();
		uint8 tgtnodeid = Nan::To<Number>(info[2]).ToLocalChecked()->Value();
		uint8 instanceid = 0;
		if(info.Length() > 3) {
			instanceid = Nan::To<Number>(info[3]).ToLocalChecked()->Value();
		}

		OZWManager( RemoveAssociation,
			homeid, nodeid, groupidx, tgtnodeid, instanceid
		);
	}

#ifdef OPENZWAVE_16
	/*
	 *
	 */
	// ===================================================================
	NAN_METHOD(OZW::IsMultiInstance)
	// ===================================================================
	{
		Nan::HandleScope scope;
		bool isMultiInstance = false;

		CheckMinArgs(2, "nodeid, groupidx");
		uint8 nodeid = Nan::To<Number>(info[0]).ToLocalChecked()->Value();
		uint8 groupidx = Nan::To<Number>(info[1]).ToLocalChecked()->Value();

		OZWManagerAssign(isMultiInstance, IsMultiInstance, homeid, nodeid, groupidx);
		info.GetReturnValue().Set(Nan::New<Boolean>(isMultiInstance));
	}
#endif

}
