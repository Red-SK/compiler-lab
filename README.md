# 食用指南

（本实验为FZU的compiler-lab）

## 开发环境

- GCC version 9.3.0（编译失败优先考虑版本问题，其中有用到C++11的语法）
- WSL Ubuntu 20.04
- 编译工具：make（如果是Windows的话，有装MinGW也可以）

## 编译

在项目根目录下

```sh
make
```

## 运行

### 默认方式

默认使用的是/test目录下的`test.lex`文件作为测试文件，在项目根目录下

```sh
./main
```

即可

### 自定义方式

如果要自己定义文件，可以自行创建，在执行时输入路径即可：（假设我在test下新建了文件`new.lex`）

```sh
./main test/new.lex
```

## 清理

若要重新编译，清除`.o`文件和`main`文件，输入：

```sh
make clean
```

# 实验内容

## 词法分析✔

### 关键字

- int
- real
- bool

- true
- false
- if
- else if 
- else
- while
- break

### 标识符

变量名（形如real num中的`num`）

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
- 支持`//`的行注释

## 语法分析✔

### LR(1)的尝试

文法G[Program]如下：（大写字母开头为非终结符，小写字母开头为终结符）

```grammar
Program   → Block
Block     → { Decls Stmts }
Decls     → Decls Decl | ε
Decl      → Type id;
Type      → Type[int_num] | Type[real_num] | int | real | bool
Stmts     → Stmts Stmt | ε
Stmt      → Var=Bool;
          | if(Bool) Stmt
          | if(Bool) Stmt else Stmt
          | while(Bool) Stmt
          | break;
          | Block
Var      → Var[num] | id
Bool     → Bool||Join | Join
Join     → Join&&Equality | Equality
Equality → Equality==Rel | Equality!=Rel | Rel
Rel      → Expr<Expr | Expr<=Expr | Expr>=Expr | Expr>Expr | Expr
Expr     → Expr+Term | Expr-Term | Term
Term     → Term*Unary | Term/Unary | Unary
Unary	 → !Unary | -Unary | Factor
Factor	 → (Bool) | Var | int_num | real_num | true | false
```

#### 文法配置文件

文法文件在/parser下的production.list，可以自行修改

程序实现了**按行读取**解析文法，所有符号之间需要用空格隔开，左部和右部用`:`分开，比如：

```
Program : Block
Block : { Decls Stmts }
```

### 文法设计：LL(1) ❌

这是一个类C的文法，我们目前打算用LL(1)

文法G[Program]如下：（大写字母开头为非终结符，小写字母开头为终结符）

```grammar
Program   → Block
Block     → { Decls Stmts }
Decls     → Decl Decls | ε
Decl      → Type id;
Type      → bool Type' | int Type' | real Type'
Type'     → [num]Type' | ε
Stmts     → Stmt Stmts | ε
Stmt      → Var = Bool; 
          | if(Bool) Block IfStmt
          | while(Bool) Stmt
          | break;
          | Block
IfStmt    → else Block | ε
Var 	  → id Var'
Var'      → [num]Var' | ε
Bool      → Join Bool'
Bool'     → || Bool | ε
Join      → Equality Join'
Join'     → && Join | ε
Equality  → Rel Equality'
Equality' → == Equality | != Equality | ε
Rel 	  → Expr Rel'
Rel' 	  → <Rel2 | >Rel2 | ε
Rel2 	  → Expr | =Expr
Expr 	  → Term Expr'
Expr'	  → +Expr | -Expr | ε
Term 	  → Unary Term'
Term'	  → *Term | /Term | ε
Unary	  → !Unary | -Unary | Factor
Factor	  → (Bool) | Var | num | true | false
```

#### FIRST集

| 非终结符  |             FIRST集             |
| :-------: | :-----------------------------: |
|  Program  |             {  {  }             |
|   Block   |             {  {  }             |
|   Decls   |      {  int,real,bool,ε  }      |
|   Decl    |       {  int,real,bool  }       |
|   Type    |       {  int,real,bool  }       |
|   Type'   |            {  [,ε  }            |
|   Stmts   |   {  id,if,while,break,{,ε  }   |
|   Stmt    |    {  id,if,while,break,{  }    |
| IfStmt(x) |          {  else,ε  }           |
|    Var    |            {  id  }             |
|  Var'(x)  |            {  [,ε  }            |
|   Bool    |  {  !,-,(,id,num,true,false  }  |
|   Bool'   |          {  \|\|,ε  }           |
|   Join    |  {  !,-,(,id,num,true,false  }  |
|   Join'   |           {  &&,ε  }            |
| Equality  |  {  !,-,(,id,num,true,false  }  |
| Equality' |           {  =,!,ε  }           |
|    Rel    |  {  !,-,(,id,num,true,false  }  |
|   Rel'    |           {  <,>,ε  }           |
|   Rel2    | {  !,-,(,id,num,true,false,=  } |
|   Expr    |  {  !,-,(,id,num,true,false  }  |
|   Expr'   |           {  +,-,ε  }           |
|   Term    |  {  !,-,(,id,num,true,false  }  |
|   Term'   |           {  *,/,ε  }           |
|   Unary   |  {  !,-,(,id,num,true,false  }  |
|  Factor   |    {  (,id,num,true,false  }    |

#### FOLLOW集

| 非终结符  |                 FOLLOW集                  |
| :-------: | :---------------------------------------: |
|  Program  |                  {  $  }                  |
|   Block   |    {  $,else,id,if,while,break,{,}  }     |
|   Decls   |        {  id,if,while,break,{,}  }        |
|   Decl    | {  int,real,bool,id,if,while,break,{,}  } |
|   Type    |                 {  id  }                  |
|   Type'   |                 {  id  }                  |
|   Stmts   |                  {  }  }                  |
|   Stmt    |     {  id,if,while,break,{,},else  }      |
| IfStmt(x) |     {  id,if,while,break,{,},else  }      |
|    Var    |  {  *,/,+,-,<,>, =,!,&&,\|\|,;,),[,id  }  |
|  Var'(x)  |  {  *,/,+,-,<,>, =,!,&&,\|\|,;,),[,id  }  |
|   Bool    |              {  ;,),[,id  }               |
|   Bool'   |              {  ;,),[,id  }               |
|   Join    |            {  \|\|,;,),[,id  }            |
|   Join'   |            {  \|\|,;,),[,id  }            |
| Equality  |          {  &&,\|\|,;,),[,id  }           |
| Equality' |          {  &&,\|\|,;,),[,id  }           |
|    Rel    |        {  =,!,&&,\|\|,;,),[,id  }         |
|   Rel'    |        {  =,!,&&,\|\|,;,),[,id  }         |
|   Rel2    |        {  =,!,&&,\|\|,;,),[,id  }         |
|   Expr    |      {  <,>, =,!,&&,\|\|,;,),[,id  }      |
|   Expr'   |      {  <,>, =,!,&&,\|\|,;,),[,id  }      |
|   Term    |    {  +,-,<,>, =,!,&&,\|\|,;,),[,id  }    |
|   Term'   |    {  +,-,<,>, =,!,&&,\|\|,;,),[,id  }    |
|   Unary   |  {  *,/,+,-,<,>, =,!,&&,\|\|,;,),[,id  }  |
|  Factor   |  {  *,/,+,-,<,>, =,!,&&,\|\|,;,),[,id  }  |

很不幸`var'`和`IfStmt`冲突了，淦，只能尝试LR(1)

## 语法制导翻译✔

## 中间代码生成

### 未完成

- 数组



 