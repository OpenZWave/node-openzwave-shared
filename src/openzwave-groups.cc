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
	 * Gets the number of association groups reported by this node. 
	 * In Z-Wave, groups are numbered starting from one. For example, 
	 * if a call to GetNumGroups returns 4, the _groupIdx value to use
	 * in calls to GetAssociations, AddAssociation and RemoveAssociation 
	 * will be a number between 1 and 4. 
	*/
	// ===================================================================
	NAN_METHOD(GetNumGroups)
	// ===================================================================
	{
		NanScope();

		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8 numGroups = OpenZWave::Manager::Get()->GetNumGroups(homeid, nodeid);
		
		NanReturnValue(NanNew<Integer>(numGroups));
	}
	
	/*
	 * 
	 */
	// ===================================================================
	NAN_METHOD(GetAssociations)
	// ===================================================================
	{
		NanScope();
		uint8* associations;

		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t groupidx = args[1]->ToNumber()->Value();
		
		uint32 numNodes = OpenZWave::Manager::Get()->GetAssociations(
			homeid, nodeid,	groupidx, &associations
		);
		
		Local<Array> o_assocs = NanNew<Array>(numNodes);

		for (uint8 nr = 0; nr < numNodes; nr++) {
			o_assocs->Set(NanNew<Integer>(nr), NanNew<Integer>(associations[nr]));
		}

		// The caller is responsible for freeing the array memory 
		// with a call to delete []. 
		delete associations;
		
		NanReturnValue(o_assocs);
	}

	/*
	 * 
	 */
	// ===================================================================
	NAN_METHOD(GetMaxAssociations)
	// ===================================================================
	{
		NanScope();
		
		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t groupidx = args[1]->ToNumber()->Value();
		
		uint8 numMaxAssoc = OpenZWave::Manager::Get()->GetMaxAssociations(
			homeid, nodeid,	groupidx
		);
		
		NanReturnValue(NanNew<Integer>(numMaxAssoc));
	}

	/*
	 * 
	 */
	// ===================================================================
	NAN_METHOD(GetGroupLabel)
	// ===================================================================
	{
		NanScope();
		
		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t groupidx = args[1]->ToNumber()->Value();

		std::string groupLabel = OpenZWave::Manager::Get()->GetGroupLabel(
			homeid, nodeid, groupidx
		);

		NanReturnValue(NanNew<String>(groupLabel.c_str()));
	}

	/*
	 * 
	 */
	// ===================================================================
	NAN_METHOD(AddAssociation)
	// ===================================================================
	{
		NanScope();
		
		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t groupidx = args[1]->ToNumber()->Value();
		uint8_t tgtnodeid = args[2]->ToNumber()->Value();
	
		OpenZWave::Manager::Get()->AddAssociation(
			homeid,nodeid,groupidx,tgtnodeid
		);

		NanReturnUndefined();
	}

	/*
	 * 
	 */
	// ===================================================================
	NAN_METHOD(RemoveAssociation)
	// ===================================================================
	{
		NanScope();
		
		uint8_t nodeid = args[0]->ToNumber()->Value();
		uint8_t groupidx = args[1]->ToNumber()->Value();
		uint8_t tgtnodeid = args[2]->ToNumber()->Value();
	
		OpenZWave::Manager::Get()->RemoveAssociation(homeid,nodeid,groupidx,tgtnodeid);
		
		NanReturnUndefined();
	}

}
