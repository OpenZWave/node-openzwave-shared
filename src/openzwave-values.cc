/*
* Copyright (c) 2013 Jonathan Perkin <jonathan@perkin.org.uk>
* Copyright (c) 2015-1016 Elias Karakoulakis <elias.karakoulakis@gmail.com>
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

		OpenZWave::ValueID* vit = populateValueId(info);
		if (vit == NULL) {
			Nan::ThrowTypeError("setValue: OpenZWave valueId not found");
		} else {
			OpenZWave::Manager* mgr = OpenZWave::Manager::Get();
			uint8 validx  =  (info[0]->IsObject()) ? 1 : 4;
			switch ((*vit).GetType()) {
				case OpenZWave::ValueID::ValueType_Bool: {
					bool val = info[validx]->BooleanValue();
					mgr->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Byte: {
					uint8 val = info[validx]->ToInteger()->Value();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Decimal: {
					float val = info[validx]->ToNumber()->NumberValue();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Int: {
					int32 val = info[validx]->ToInteger()->Value();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_List: {
					std::string val = (*String::Utf8Value(info[validx]->ToString()));
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Short: {
					int16 val = info[validx]->ToInteger()->Value();
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_String: {
					std::string val = (*String::Utf8Value(info[validx]->ToString()));
					OpenZWave::Manager::Get()->SetValue(*vit, val);
					break;
				}
				case OpenZWave::ValueID::ValueType_Schedule: {
					Nan::ThrowTypeError("please use the specialized xxxSwitchPoint commands instead of setValue for setting thermostat schedules");
					break;
				}
				case OpenZWave::ValueID::ValueType_Button: {
					OpenZWave::Manager::Get()->PressButton(*vit);
					break;
				}
				case OpenZWave::ValueID::ValueType_Raw: {
					checkType(Buffer::HasInstance(info[validx]));
					uint8 *val = (uint8*)Buffer::Data(info[validx]);
					uint8 len  = Buffer::Length(info[validx]);
					OpenZWave::Manager::Get()->SetValue(*vit, val, len);
					break;
				}
			}
		}
	}


/*
*
*/

/*
* Refresh value from Z-Wave network
*/
// =================================================================
NAN_METHOD(OZW::RefreshValue)
    // =================================================================
{
	Nan::HandleScope scope;

	OpenZWave::ValueID* vit = populateValueId(info);
	if (vit == NULL) {
		Nan::ThrowTypeError("refreshValue: OpenZWave valueId not found");
	} else {
		OpenZWave::Manager* mgr = OpenZWave::Manager::Get();
		const bool ok = mgr->RefreshValue(*vit);
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

	OpenZWave::ValueID* vit = populateValueId(info);
	if (vit == NULL) {
		Nan::ThrowTypeError("setChangeVerified: OpenZWave valueId not found");
	} else {
		const uint8 validx = (info[0]->IsObject()) ? 1 : 4;
		if (checkType(info[validx]->IsBoolean())) {
			OpenZWave::Manager* mgr = OpenZWave::Manager::Get();
			mgr->SetChangeVerified(*vit, info[validx]->BooleanValue());
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

	OpenZWave::ValueID* vit = populateValueId(info);
	if ((vit == NULL) || ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ))  {
		Nan::ThrowTypeError("OpenZWave valueId not found or not of the correct type");
	} else {
		info.GetReturnValue().Set(Nan::New<Integer>(
			OpenZWave::Manager::Get()->GetNumSwitchPoints(*vit)
		));
	}
}

// =================================================================
NAN_METHOD(OZW::GetSwitchPoint)
// =================================================================
{
	Nan::HandleScope scope;
	uint8 idx, o_hours, o_minutes;
	int8  o_setback;

	OpenZWave::ValueID* vit = populateValueId(info);
	if ((vit == NULL) || ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ))  {
		Nan::ThrowTypeError("OpenZWave valueId not found or not of the correct type");
	} else {
		uint8 idxpos  =  (info[0]->IsObject()) ? 1 : 4;
		if ((info.Length() < idxpos) || !info[idxpos]->IsNumber()) {
			Nan::ThrowTypeError("must supply an integer index after the valueId");
		} else {
			idx = info[idxpos]->ToNumber()->Value();
			OpenZWave::Manager::Get()->GetSwitchPoint(*vit, idx, &o_hours, &o_minutes, &o_setback);
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

	OpenZWave::ValueID* vit = populateValueId(info);
	if ((vit == NULL) || ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ))  {
		Nan::ThrowTypeError("OpenZWave valueId not found or not of the correct type");
	} else {
		OpenZWave::Manager::Get()->ClearSwitchPoints(*vit);
	}
}

// =================================================================
NAN_METHOD(OZW::SetSwitchPoint)
// =================================================================
{
	Nan::HandleScope scope;

	OpenZWave::ValueID* vit = populateValueId(info);
	if ((vit == NULL) || ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ))  {
		Nan::ThrowTypeError("OpenZWave valueId not found or not of the correct type");
	} else {
		uint8 idxpos  =  (info[0]->IsObject()) ? 1 : 4;
		if ((info.Length() < idxpos) || !info[idxpos]->IsObject()) {
			Nan::ThrowTypeError("must supply a switchpoint object ");
		} else {
			Local<Object> sp = info[idxpos]->ToObject();
			OpenZWave::Manager::Get()->SetSwitchPoint(*vit,
				Nan::Get(sp, Nan::New<String>("hours").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value(),
				Nan::Get(sp, Nan::New<String>("minutes").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value(),
				Nan::Get(sp, Nan::New<String>("setback").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value()
			);
		}
	}
}

// =================================================================
NAN_METHOD(OZW::RemoveSwitchPoint)
// =================================================================
{
	Nan::HandleScope scope;

	OpenZWave::ValueID* vit = populateValueId(info);
	if ((vit == NULL) || ((*vit).GetType() != OpenZWave::ValueID::ValueType_Schedule ))  {
		Nan::ThrowTypeError("OpenZWave valueId not found or not of the correct type");
	} else {
		uint8 idxpos  =  (info[0]->IsObject()) ? 1 : 4;
		if ((info.Length() < idxpos) || !info[idxpos]->IsObject()) {
			Nan::ThrowTypeError("must supply a switchpoint object ");
		} else {
			Local<Object> sp = info[idxpos]->ToObject();
			OpenZWave::Manager::Get()->RemoveSwitchPoint(*vit,
				Nan::Get(sp, Nan::New<String>("hours").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value(),
				Nan::Get(sp, Nan::New<String>("minutes").ToLocalChecked()).ToLocalChecked()->ToNumber()->Value()
			);
		}
	}
}

}
