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

1.  xxxx
2.  xxxx
3.  xxxx

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
