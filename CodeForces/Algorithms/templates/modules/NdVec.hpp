#pragma once
#include "templates/core/ContainerAliases.hpp"

//=====----- [ NdVec ] --------------------------------------------------=====//

template <class T>
auto make_vec2(Size n1, Size n2) { return Vec(n1, Vec<T>(n2)); }

template <class T>
auto make_vec2(Size n1, Size n2, const T& value) { return Vec(n1, Vec<T>(n2, value)); }

template <class T>
auto make_vec3(Size n1, Size n2, Size n3) { return Vec(n1, Vec(n2, Vec<T>(n3))); }

template <class T>
auto make_vec3(Size n1, Size n2, Size n3, const T& value) {
  return Vec(n1, Vec(n2, Vec<T>(n3, value)));
}

template <class T>
auto make_vec4(Size n1, Size n2, Size n3, Size n4) {
  return Vec(n1, Vec(n2, Vec(n3, Vec<T>(n4))));
}

template <class T>
auto make_vec4(Size n1, Size n2, Size n3, Size n4, const T& value) {
  return Vec(n1, Vec(n2, Vec(n3, Vec<T>(n4, value))));
}
