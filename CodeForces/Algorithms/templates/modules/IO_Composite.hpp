#ifndef CP_IO_COMPOSITE_CONTEXT
  #error "IO_Composite.hpp is an implementation detail: include templates/modules/IO.hpp \
          or templates/modules/Fast_IO.hpp instead."
#endif

//=====----- [ I/O Composite ] ------------------------------------------=====//

template <class T, class U>
void read(std::pair<T, U>&);
template <class T, class A>
void read(std::vector<T, A>&);
template <class... Args>
void read(std::tuple<Args...>&);

template <class T, class U>
void write(const std::pair<T, U>&);
template <class T, class A>
void write(const std::vector<T, A>&);
template <class... Args>
void write(const std::tuple<Args...>&);

template <class T, class U>
inline void read(std::pair<T, U>& p) {
  read(p.first);
  read(p.second);
}

template <class T, class A>
inline void read(std::vector<T, A>& v) {
  for (auto& x : v)
    read(x);
}

template <class... Args>
inline void read(std::tuple<Args...>& t) {
  std::apply([](auto&... args) { (read(args), ...); }, t);
}

template <class T, class U>
inline void write(const std::pair<T, U>& p) {
  write(p.first);
  write(' ');
  write(p.second);
}

template <class T, class A>
inline void write(const std::vector<T, A>& v) {
  for (std::size_t i = 0; i < v.size(); ++i) {
    if (i)
      write(' ');
    write(v[i]);
  }
}

template <class... Args>
inline void write(const std::tuple<Args...>& t) {
  bool first = true;
  std::apply(
      [&first](const auto&... args) {
        ((first ? (first = false, void()) : write(' '), write(args)), ...);
      },
      t);
}
