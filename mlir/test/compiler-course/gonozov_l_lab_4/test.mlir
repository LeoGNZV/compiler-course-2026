// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/FunctionCallCounterPass%shlibext --pass-pipeline="builtin.module(FunctionCallCounterPass)" %s | FileCheck %s

// Проверяем, что пасс правильно подсчитал вызовы
// CHECK: Функция 'isEven' вызвана 0 раз(а)
// CHECK: Функция 'main' вызвана 0 раз(а)
// CHECK: Функция 'helper' вызвана 2 раз(а)
// CHECK: Количество операций: {{[0-9]+}}
// CHECK-NEXT: module attributes {{.*}} 
// CHECK: func.func @isEven([[arg0:%.+]]: i32) -> i1 attributes {call_count = 0 : i32} 
// CHECK: func.func @main() attributes {call_count = 0 : i32} 
// CHECK: func.func @helper([[arg0:%.+]]: i32) -> i32 attributes {call_count = 2 : i32} 

module {
  // Функция, которая никогда не вызывается
  // Так как на неё нет операций func.call, счётчик будет 0
  func.func @isEven(%arg0: i32) -> i1 {
    %0 = arith.constant 1 : i32
    %1 = arith.constant 0 : i32
    %2 = arith.andi %arg0, %0 : i32
    %3 = arith.cmpi eq, %2, %1 : i32
    func.return %3 : i1
  }
  
  // Вспомогательная функция, которая будет вызвана дважды
  func.func @helper(%arg0: i32) -> i32 {
    %0 = arith.constant 1 : i32
    %1 = arith.addi %arg0, %0 : i32
    func.return %1 : i32
  }
  
  // Главная функция, которая дважды вызывает helper
  func.func @main() {
    %0 = arith.constant 5 : i32
    %1 = func.call @helper(%0) : (i32) -> i32  // Первый вызов helper
    %2 = arith.constant 10 : i32
    %3 = func.call @helper(%2) : (i32) -> i32  // Второй вызов helper
    func.return
  }
}
