#include "Response.h"

Response::Response(uint32_t payload_size) {
	this->payload_size = payload_size;
}

Response::~Response() {}