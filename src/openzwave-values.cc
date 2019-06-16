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
	* Generic value set.
	*/
	// =================================================================
	NAN_METHOD(OZW::SetValue)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "valueId");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			uint8 validx  =  (info[0]->IsObject()) ? 1 : 4;
			switch ((*vit).GetType()) {
				case OpenZWave::ValueID::ValueType_Bool: {
					bool val = Nan::To<Boolean>(info[validx]).ToLocalChecked()->Value();
					OZWManager( SetValue, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Byte: {
					uint8 val = Nan::To<Integer>(info[validx]).ToLocalChecked()->Value();
					OZWManager( SetValue, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Decimal: {
					float val = Nan::To<Number>(info[validx]).ToLocalChecked()->Value();
					OZWManager( SetValue, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Int: {
					int32 val = Nan::To<Integer>(info[validx]).ToLocalChecked()->Value();
					OZWManager( SetValue, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_List: {
					::std::string val(*Nan::Utf8String( info[validx] ));
					OZWManager( SetValue, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Short: {
					int16 val = Nan::To<Integer>(info[validx]).ToLocalChecked()->Value();
					OZWManager( SetValue, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_String: {
					::std::string val(*Nan::Utf8String( info[validx] ));
					OZWManager( SetValue, *vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Schedule: {
					Nan::ThrowTypeError("please use the specialized xxxSwitchPoint commands instead of setValue for setting thermostat schedules");
					break;
				}
				case OpenZWave::ValueID::ValueType_Button: {
					if (checkType(info[validx]->IsBoolean())) {
						if (Nan::To<Boolean>(info[validx]).ToLocalChecked()->Value()) {
							OZWManager( PressButton, *vit);
						} else {
							OZWManager( ReleaseButton, *vit);
						}
					}
					break;
				}
				case OpenZWave::ValueID::ValueType_Raw: {
					checkType(Buffer::HasInstance(info[validx]));
					uint8 *val = (uint8*)Buffer::Data(info[validx]);
					uint8 len  = Buffer::Length(info[validx]);
					OZWManager( SetValue, *vit, val, len);
					break;
				}
#if OPENZWAVE_16
				case OpenZWave::ValueID::ValueType_BitSet: {
					uint8 pos = Nan::To<Number>(info[validx]).ToLocalChecked()->Value();
					bool val = Nan::To<Boolean>(info[validx+1]).ToLocalChecked()->Value();
					OZWManager( SetValue, *vit, pos, val);
				}
#endif
			}
		}
	}

	// =================================================================
	NAN_METHOD(OZW::SetValueLabel)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(2, "valueid, label");
		OpenZWave::ValueID* vit = populateValueId(info);
		uint8 validx  =  (info[0]->IsObject()) ? 1 : 4;
		::std::string label(*Nan::Utf8String( info[validx] ));

		if (vit) {
			OZWManager( SetValueLabel, *vit, label);
		}
	}

	/*
	* Refresh value from Z-Wave network
	*/
	// =================================================================
	NAN_METHOD(OZW::RefreshValue)
			// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "valueId");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			bool ok = false;
			OZWManagerAssign(ok, RefreshValue, *vit);
			info.GetReturnValue().Set(Nan::New<Boolean>(ok));
		}
	}

	/*
	* Ask OZW to verify the value change before notifying the application
	*/
	// =================================================================
	NAN_METHOD(OZW::SetChangeVerified)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "valueId");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			const uint8 validx = (info[0]->IsObject()) ? 1 : 4;
			if (checkType(info[validx]->IsBoolean())) {
				bool b = Nan::To<Boolean>(info[validx]).ToLocalChecked()->Value();
				OZWManager( SetChangeVerified, *vit, b);
			}
		}
	}

	/*
	* Get number of thermostat switch points
	*/
	// =================================================================
	NAN_METHOD(OZW::GetNumSwitchPoints)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "valueId");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule)  {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_Schedule");
			}
			uint8 i = -1;
			OZWManagerAssign(i, GetNumSwitchPoints, *vit );
			info.GetReturnValue().Set(Nan::New<Integer>(i));
		}
	}

	// =================================================================
	NAN_METHOD(OZW::GetSwitchPoint)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "valueId");
		uint8 idx, o_hours, o_minutes;
		int8  o_setback;

		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ) {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_Schedule");
			}
			uint8 idxpos  =  (info[0]->IsObject()) ? 1 : 4;
			if ((info.Length() < idxpos) || !info[idxpos]->IsNumber()) {
				Nan::ThrowTypeError("must supply an integer index after the valueId");
			} else {
				idx = Nan::To<Number>(info[idxpos]).ToLocalChecked()->Value();
				OZWManager( GetSwitchPoint, *vit, idx, &o_hours, &o_minutes, &o_setback);
				Local<Object> o  = Nan::New<Object>();
				Nan::Set(o,
					Nan::New<String>("hours").ToLocalChecked(),
					Nan::New<Integer>(o_hours));
				Nan::Set(o,
					Nan::New<String>("minutes").ToLocalChecked(),
					Nan::New<Integer>(o_minutes));
				Nan::Set(o,
					Nan::New<String>("setback").ToLocalChecked(),
					Nan::New<Integer>(o_setback));
				info.GetReturnValue().Set(o);
			}
		}
	}

	// Clears all switch points from the schedule.
	// =================================================================
	NAN_METHOD(OZW::ClearSwitchPoints)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "valueId");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ) {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_Schedule");
			} else {
				OZWManager( ClearSwitchPoints, *vit);
			}
		}
	}

	// =================================================================
	NAN_METHOD(OZW::SetSwitchPoint)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "valueId");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ) {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_Schedule");
			} else {
				uint8 idxpos  =  (info[0]->IsObject()) ? 1 : 4;
				if ((info.Length() < idxpos) || !info[idxpos]->IsObject()) {
					Nan::ThrowTypeError("must supply a switchpoint object");
				} else {
					Nan::MaybeLocal<v8::Object> sp_maybe = Nan::To<v8::Object>(info[idxpos]);
					if (sp_maybe.IsEmpty()) return;
					v8::Local<v8::Object> sp = sp_maybe.ToLocalChecked();
					OZWManager( SetSwitchPoint, *vit,
						Nan::To<Number>(Nan::Get(sp, Nan::New<String>("hours").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value(),
						Nan::To<Number>(Nan::Get(sp, Nan::New<String>("minutes").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value(),
						Nan::To<Number>(Nan::Get(sp, Nan::New<String>("setback").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value()
					);
				}
			}
		}
	}

	// =================================================================
	NAN_METHOD(OZW::RemoveSwitchPoint)
	// =================================================================
	{
		Nan::HandleScope scope;
		CheckMinArgs(1, "valueId");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ) {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_Schedule");
			} else {
				uint8 idxpos  =  (info[0]->IsObject()) ? 1 : 4;
				if ((info.Length() < idxpos) || !info[idxpos]->IsObject()) {
					Nan::ThrowTypeError("must supply a switchpoint object ");
				} else {
					Local<Object> sp = Nan::To<Object>(info[idxpos]).ToLocalChecked();
					OZWManager( RemoveSwitchPoint, *vit,
						Nan::To<Number>(Nan::Get(sp, Nan::New<String>("hours").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value(),
						Nan::To<Number>(Nan::Get(sp, Nan::New<String>("minutes").ToLocalChecked()).ToLocalChecked()).ToLocalChecked()->Value()
					);
				}
			}

		}
	}

#if OPENZWAVE_16
	// =================================================================
	NAN_METHOD(OZW::GetValueAsBitSet)
	// =================================================================
	{
		CheckMinArgs(2, "valueid, pos");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_BitSet ) {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_BitSet");
			}
			uint8 idxpos  =  (info[0]->IsObject()) ? 1 : 4;
			if ((info.Length() < idxpos) || !info[idxpos]->IsNumber()) {
				Nan::ThrowTypeError("must supply an integer for _pos after the valueId");
			} else {
				bool o_value = false;
				uint8 _pos = Nan::To<Number>(info[idxpos]).ToLocalChecked()->Value();
				OZWManager(GetValueAsBitSet, *vit, _pos, &o_value);
				info.GetReturnValue().Set(Nan::New<Boolean>(o_value));
			}
		}
	}

	// =================================================================
	NAN_METHOD(OZW::SetBitMask)
	// =================================================================
	{
		CheckMinArgs(2, "valueid, mask");
		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_BitSet ) {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_BitSet");
			}
			uint8 idxpos  =  (info[0]->IsObject()) ? 1 : 4;
			if ((info.Length() < idxpos) || !info[idxpos]->IsNumber()) {
				Nan::ThrowTypeError("must supply an integer for _mask after the valueId");
			} else {
				uint32 _mask = Nan::To<Number>(info[idxpos]).ToLocalChecked()->Value();
				OZWManager(SetBitMask, *vit, _mask);
			}
		}
	}

	// =================================================================
	NAN_METHOD(OZW::GetBitMask)
	// =================================================================
	{
		CheckMinArgs(1, "valueid");
		OpenZWave::ValueID* vit = populateValueId(info);
		int32 o_mask = 0;
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_BitSet ) {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_BitSet");
			}
			OZWManager(GetBitMask, *vit, &o_mask);
			info.GetReturnValue().Set(Nan::New<Number>(o_mask));
		}
	}

	// =================================================================
	NAN_METHOD(OZW::GetBitSetSize)
	// =================================================================
	{
		CheckMinArgs(1, "valueid");
		OpenZWave::ValueID* vit = populateValueId(info);
		uint8 o_size = 0;
		if (vit) {
			if ((*vit).GetType() != OpenZWave::ValueID::ValueType_BitSet ) {
				Nan::ThrowTypeError("OpenZWave valueId is not a ValueType_BitSet");
			}
			OZWManager(GetBitSetSize, *vit, &o_size);
			info.GetReturnValue().Set(Nan::New<Number>(o_size));
		}
	}
#endif
}
