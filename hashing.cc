#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <stdint.h>
#include <iostream>
#include "nan.h"

// Main Imports
#include "algorithms/sha256d/sha256d.h"
#include "algorithms/firopow/firopow.h"
#include "algorithms/firopow/firopow.hpp"
#include "algorithms/firopow/firopow_progpow.hpp"
#include "algorithms/main/x10/x10.h"

using namespace node;
using namespace v8;

#define THROW_ERROR_EXCEPTION(x) Nan::ThrowError(x)
const char* ToCString(const Nan::Utf8String& value) {
  return *value ? *value : "<string conversion failed>";
}

// Firopow Algorithm
NAN_METHOD(firopow) {

  // Check Arguments for Errors
  if (info.Length() < 5)
    return THROW_ERROR_EXCEPTION("You must provide five arguments.");

  // Process/Define Passed Parameters [1]
  const ethash::hash256* header_hash_ptr = (ethash::hash256*)Buffer::Data(Nan::To<v8::Object>(info[0]).ToLocalChecked());
  uint64_t* nonce64_ptr = (uint64_t*)Buffer::Data(Nan::To<v8::Object>(info[1]).ToLocalChecked());
  int block_height = info[2]->IntegerValue(Nan::GetCurrentContext()).FromJust();
  const ethash::hash256* mix_hash_ptr = (ethash::hash256*)Buffer::Data(Nan::To<v8::Object>(info[3]).ToLocalChecked());
  ethash::hash256* hash_out_ptr = (ethash::hash256*)Buffer::Data(Nan::To<v8::Object>(info[4]).ToLocalChecked());

  // Process/Define Passed Parameters [2]
  static firopow_main::epoch_context_ptr context{nullptr, nullptr};
  const auto epoch_number = firopow_main::get_epoch_number(block_height);
  if (!context || context->epoch_number != epoch_number)
      context = firopow_main::create_epoch_context(epoch_number);

  // Hash Input Data and Check if Valid Solution
  bool is_valid = firopow_progpow::verify(*context, block_height, header_hash_ptr, *mix_hash_ptr, *nonce64_ptr, hash_out_ptr);
  if (is_valid) info.GetReturnValue().Set(Nan::True());
  else info.GetReturnValue().Set(Nan::False());
}

// Sha256d Algorithm
NAN_METHOD(sha256d) {

  // Check Arguments for Errors
  if (info.Length() < 1)
    return THROW_ERROR_EXCEPTION("You must provide one argument.");

  // Process/Define Passed Parameters
  char * input = Buffer::Data(Nan::To<v8::Object>(info[0]).ToLocalChecked());
  uint32_t input_len = Buffer::Length(Nan::To<v8::Object>(info[0]).ToLocalChecked());
  char output[32];

  // Hash Input Data and Return Output
  sha256d_hash(input, output, input_len);
  info.GetReturnValue().Set(Nan::CopyBuffer(output, 32).ToLocalChecked());
}

// X10 Algorithm
NAN_METHOD(x10) {

  // Check Arguments for Errors
  if (info.Length() < 1)
    return THROW_ERROR_EXCEPTION("You must provide one argument.");

  // Process/Define Passed Parameters
  char * input = Buffer::Data(Nan::To<v8::Object>(info[0]).ToLocalChecked());
  uint32_t input_len = Buffer::Length(Nan::To<v8::Object>(info[0]).ToLocalChecked());
  char output[32];

  // Hash Input Data and Return Output
  x10_hash(input, output, input_len);
  info.GetReturnValue().Set(Nan::CopyBuffer(output, 32).ToLocalChecked());
}


NAN_MODULE_INIT(init) {
  Nan::Set(target, Nan::New("firopow").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(firopow)).ToLocalChecked());
  Nan::Set(target, Nan::New("sha256d").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(sha256d)).ToLocalChecked());
  Nan::Set(target, Nan::New("x10").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(x10)).ToLocalChecked());
}

NODE_MODULE(hashing, init)
