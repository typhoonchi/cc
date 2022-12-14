# CMinus编译器

#### 介绍
实现一个简易CMinus编译器的前后端, 参考自 C4 开源项目

#### 软件架构
  本项目采用模块化的思想, 根据项目实现的功能将整个文件划分为词法分析模块、语法分析模块、代码生成模块、代码运行模块以及实现初始化和可视化等功能的辅助功能模块共计五个模块.

词法分析模块使用 DFA 进行分析, 并输出词素类型以及词素在符号表中的位置, 通过两个全局变量传递给其余模块进行后续的分析. 语法分析模块接收词法分析模块传入的结果, 使用递归下降进行分析, 并结合符号表进行简单的语义检查, 最终输出一棵抽象语法树 AST. 代码生成模块接收该抽象语法树, 并进行遍历, 在遍历的过程中生成虚拟机代码. 代码运行模块会找到 main 函数入口处, 并运行代码.

此外, 定义全局头文件作为不同模块之间的通信的桥梁, 其中存放所有模块都会使用的枚举变量、结构体以及全局变量.


#### 安装教程

1. 在命令行中进入到项目根目录, 运行 cmake 文件构建可执行程序, 或者使用 cmake 命令生成 Makefile 文件再使用 make 命令构件可执行程序.
2. 使用 ./cMinusCompiler 运行程序, 后面跟要编译的程序. 如:
```
   ./cMinusCompiler ./sample/test
```


#### 使用说明 
- 声明语句:
1. 单条语句内声明一个或多个同类型 (int, char) 的基本变量, 一维数组变量或多维数组变量;

2. 支持十进制、八进制以及十六进制整数;

3. 不允许在声明的同时进行初始化操作;

4. 定义返回值为 int, char, void 类型的函数; 函数参数列表可以有数组, 但是不能用指针的形式表示; 参数列表为空时可以是一对空括号, 也可以是内部有 void 的一对括号;

5. 函数内局部变量声明不要求一次性在函数开始处声明, 但是不能在 If Else 语句、While 语句、For 语句以及 Do While 语句内部声明;

- If Else 语句:
1. 与 C89 中 If Else 语句用法基本一致, 允许单个 If 语句, 单个 If Else 语句, 嵌套的 If Else 语句以及并列的 If Else 语句;

- While 语句:
1. 与 C89 中 While 语句用法基本一致;

- For 语句:
1. 与 C89 中 For 语句用法基本一致, 也不允许边声明边初始化;

- Do While 语句:
1. 与 C89 中 Do While 语句用法基本一致;

- 表达式语句:
1. 支持加减乘除取模、左移右移、按位与、按位或、异或、逻辑与、逻辑或、逻辑非、等于、不等于、大于、大于等于、小于、小于等于、取下标运算;
支持使用括号;

2. 支持上述运算的复合运算;

3. 不支持取地址, 解引用等运算;

- Return 语句:
1. 与 C89 中 Return 语句用法基本一致.


