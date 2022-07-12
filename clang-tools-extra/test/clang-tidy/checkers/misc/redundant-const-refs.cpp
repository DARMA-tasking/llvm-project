// RUN: %check_clang_tidy %s misc-redundant-const-refs %t

// TODO (STRZ) - test not finished yet

int foo1(const int &A) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'notAwesomeFunc' is insufficiently awesome
  // CHECK-MESSAGES: :[[@LINE-2]]:6: note: insert 'awesome'
  const int B = 2;
  return A + B;
}

int bar2(const int A) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'notAwesomeFunc' is insufficiently awesome
  // CHECK-MESSAGES: :[[@LINE-2]]:6: note: insert 'awesome'
  const int B = 2;
  return A + B;
}

int foo3(int &A) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'notAwesomeFunc' is insufficiently awesome
  // CHECK-MESSAGES: :[[@LINE-2]]:6: note: insert 'awesome'
  const int B = 2;
  return A + B;
}

template <typename TypeT>
int foo4(const TypeT &T, const int &A) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'notAwesomeFunc' is insufficiently awesome
  // CHECK-MESSAGES: :[[@LINE-2]]:6: note: insert 'awesome'
  return T.val() + A;
}

struct BigStruct {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'notAwesomeFunc' is insufficiently awesome
  // CHECK-MESSAGES: :[[@LINE-2]]:6: note: insert 'awesome'
  int Aa = 0;
  int Bb = 0;
  int Cc = 0;
  int Dd = 0;
  int Ee = 0;
};

int foo5(const BigStruct &Bs) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'notAwesomeFunc' is insufficiently awesome
  // CHECK-MESSAGES: :[[@LINE-2]]:6: note: insert 'awesome'
  return Bs.Aa - Bs.Bb + Bs.Cc + Bs.Dd + Bs.Ee;
}
