#pragma once
#include "../config.h"
namespace talos {
	class Component {
		public:

			virtual std::string getName() const = 0;
	};
}