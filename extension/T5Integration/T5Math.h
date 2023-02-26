#pragma once

namespace T5Integration {

	class T5Math {
		public:
		using Ptr = std::shared_ptr<T5Math>;

		virtual void rotate_vector(float quat_x, float quat_y, float quat_z, float quat_w, float& vec_x, float& vec_y, float& vec_z) = 0;
	};

}