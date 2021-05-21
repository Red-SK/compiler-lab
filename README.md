# 食用指南

（本实验为FZU的compiler-lab）

## 编译

在项目根目录下

```sh
make
```

## 运行

在项目根目录下

```sh
./main
```

即可

默认使用的是/test目录下的`test.lex`文件作为测试文件，如果要自己定义文件，可以自行创建，在执行时输入路径即可：（假设我在test下新建了文件`new.lex`）

```sh
./main test/new.lex
```

## 清理

若要重新编译，清除`.o`文件和`main`文件，输入：

```sh
make clean
```

# 实验内容

## 词法分析（已完成）

### 关键字

- int
- real
- bool

- true
- false
- if
- else if 
- else
- for 
- break
- return 

### 标识符

- 变量名（形如real num中的`num`）
- 函数名（形如int foo()中的`foo`）

### 常数

比如`3.14`

### 运算符

- `+`,`-`,`*`,`/`

- `=`,`==`,`!=`
- `&&`,`||`,`!`
- `>`,`>=`,`<`,`<=`

### 分隔符

- `,`,`;`
- `{`,`}`
- `[`,`]`

- `(`,`)`

## 语法分析（未完成）

## 语法制导翻译（未完成）

## 中间代码生成（未完成）



 





 