// RUN: %check_clang_tidy %s misc-redundant-const-refs %t

int constRefInt(const int &I) {
  // CHECK-MESSAGES: :[[@LINE-1]]:17: warning: passing 'int' (32 bits) by const ref, consider passing by value [misc-redundant-const-refs]
  // CHECK-FIXES: const int I
  constexpr int B = 2;
  return I + B;
}

int constInt(const int I) {
  constexpr int B = 2;
  return I + B;
}

int refInt(int &A) {
  const int B = 2;
  return A + B;
}

template <typename TypeT>
int constRefTConstRefInt(const TypeT &T, const int &I) {
  // CHECK-MESSAGES: :[[@LINE-1]]:42: warning: passing 'int' (32 bits) by const ref, consider passing by value [misc-redundant-const-refs]
  // CHECK-FIXES: const int I
  return T.val() + I;
}

struct StructTwoInts {
  int Aa;
  int Bb;
};

int constRefStructTwoInts(const StructTwoInts &Sti) {
  // CHECK-MESSAGES: :[[@LINE-1]]:27: warning: passing 'StructTwoInts' (64 bits) by const ref, consider passing by value [misc-redundant-const-refs]
  // CHECK-FIXES: const StructTwoInts Sti
  return Sti.Aa - Sti.Bb;
}

struct StructFiveInts {
  int Aa;
  int Bb;
  int Cc;
  int Dd;
  int Ee;
};

int constRefStructFiveInts(const StructFiveInts &Sfi) {
  return Sfi.Aa - Sfi.Bb + Sfi.Cc - Sfi.Dd + Sfi.Ee;
}

template <typename TypeT> struct DependentStruct {
public:
  DependentStruct(const TypeT &T) : T{T} {}
  int asInt() const { return T.asInt(); }

private:
  const TypeT T;
};

// Ignore template functions
template <typename TypeT>
int constDependentStruct(const DependentStruct<TypeT> &Ds) {
  return Ds.asInt();
}

template <> struct DependentStruct<int> {
public:
  DependentStruct(int I) : I{I} {}
  int asInt() const { return I; }

private:
  const int I;
};

// Ignore template function specialization
template <> int constDependentStruct<int>(const DependentStruct<int> &Ds) {
  return Ds.asInt();
}

int constDependentStruct(const DependentStruct<int> &Ds) {
  // CHECK-MESSAGES: :[[@LINE-1]]:26: warning: passing 'DependentStruct<int>' (32 bits) by const ref, consider passing by value [misc-redundant-const-refs]
  // CHECK-FIXES: const DependentStruct<int> Ds
  return Ds.asInt();
}

// TODO (STRZ):
// Lambda's copy constructor should be ignore.
// Look at `dyn_cast_or_null<CXXConstructorDecl>(FuncDecl)`
bool isDone(const bool IsNotDone) {
  auto Fn = [IsNotDone] { return not IsNotDone; };
  return Fn();
}

// TOOD (STRZ): that should produce warning, but is ignored because of
// const auto *FuncDecl =
//     dyn_cast_or_null<FunctionDecl>(ConstRefParm->getParentFunctionOrMethod());
// if (!FuncDecl) {
//   return;
// }
// using SomeFunction = std::function<void(const int&)>;
