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
	* Gets the neighbors for a node
	*/
	// ===================================================================
	NAN_METHOD(OZW::GetNodeNeighbors)
	// ===================================================================
	{
		OZWGETMGRNODEID;
		uint8* neighbors;
		uint8 numNeighbors = mgr->GetNodeNeighbors(homeid, nodeid, &neighbors);
		Local<Array> o_neighbors = Nan::New<Array>(numNeighbors);

		for (uint8 nr = 0; nr < numNeighbors; nr++) {
			o_neighbors->Set(Nan::New<Integer>(nr), Nan::New<Integer>(neighbors[nr]));
		}
		delete[] neighbors;

		info.GetReturnValue().Set( o_neighbors );
	}

	// =================================================================
	NAN_METHOD(OZW::SetNodeOn)               { OZWMGR1(SetNodeOn); }
	NAN_METHOD(OZW::SetNodeOff)              { OZWMGR1(SetNodeOff) }
	NAN_METHOD(OZW::SetNodeLevel)            { OZWMGRSetNodeUint8(SetNodeLevel) }
	NAN_METHOD(OZW::SwitchAllOn)             { OZWMGR0(SwitchAllOn) }
	NAN_METHOD(OZW::SwitchAllOff)            { OZWMGR0(SwitchAllOff) }
	NAN_METHOD(OZW::SetNodeLocation)         { OZWMGRSetNodeString(SetNodeLocation) }
	NAN_METHOD(OZW::SetNodeName)             { OZWMGRSetNodeString(SetNodeName) }
	NAN_METHOD(OZW::SetNodeManufacturerName) { OZWMGRSetNodeString(SetNodeManufacturerName) }
	NAN_METHOD(OZW::SetNodeProductName)      { OZWMGRSetNodeString(SetNodeProductName) }
	NAN_METHOD(OZW::RefreshNodeInfo)         { OZWMGR1(RefreshNodeInfo) }
	NAN_METHOD(OZW::RequestNodeState) 			 { OZWMGRGetBool(RequestNodeState) }
	NAN_METHOD(OZW::RequestNodeDynamic)      { OZWMGRGetBool(RequestNodeDynamic) }
	NAN_METHOD(OZW::IsNodeListeningDevice)   { OZWMGRGetBool(IsNodeListeningDevice) }
	NAN_METHOD(OZW::IsNodeFrequentListeningDevice) { OZWMGRGetBool(IsNodeFrequentListeningDevice) }
	NAN_METHOD(OZW::IsNodeBeamingDevice)     { OZWMGRGetBool(IsNodeBeamingDevice) }
	NAN_METHOD(OZW::IsNodeRoutingDevice)     { OZWMGRGetBool(IsNodeRoutingDevice) }
	NAN_METHOD(OZW::IsNodeSecurityDevice)    { OZWMGRGetBool(IsNodeSecurityDevice) }
	NAN_METHOD(OZW::GetNodeMaxBaudRate)      { OZWMGRGetUint32(GetNodeMaxBaudRate) }
	NAN_METHOD(OZW::GetNodeVersion)          { OZWMGRGetUint8(GetNodeVersion) }
	NAN_METHOD(OZW::GetNodeSecurity)         { OZWMGRGetUint8(GetNodeSecurity) }
	NAN_METHOD(OZW::GetNodeBasic)            { OZWMGRGetUint8(GetNodeBasic) }
	NAN_METHOD(OZW::GetNodeGeneric)          { OZWMGRGetUint8(GetNodeGeneric) }
	NAN_METHOD(OZW::GetNodeSpecific)         { OZWMGRGetUint8(GetNodeSpecific) }
	NAN_METHOD(OZW::GetNodeType)             { OZWMGRGetString(GetNodeType) }
	NAN_METHOD(OZW::GetNodeProductName)      { OZWMGRGetString(GetNodeProductName) }
	NAN_METHOD(OZW::GetNodeName)             { OZWMGRGetString(GetNodeName) }
	NAN_METHOD(OZW::GetNodeLocation)         { OZWMGRGetString(GetNodeLocation) }
	NAN_METHOD(OZW::GetNodeManufacturerName) { OZWMGRGetString(GetNodeManufacturerName) }
	NAN_METHOD(OZW::GetNodeManufacturerId)   { OZWMGRGetString(GetNodeManufacturerId) }
	NAN_METHOD(OZW::GetNodeProductType)      { OZWMGRGetString(GetNodeProductType) }
	NAN_METHOD(OZW::GetNodeProductId)        { OZWMGRGetString(GetNodeProductId) }
}
