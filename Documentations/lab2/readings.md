# 拓展阅读

## 不同的生成器

Bison 并非唯一的解析器生成器，甚至不是最好用的。我们推荐同学们进一步了解其他生成器，以备不时之需。

可以从以下几个角度来研究：

1. 支持怎样的文法？
2. 目标语言是什么？
3. 是如何实现的？
4. 支持怎样的 lexer？

等等。实际上，Wikipedia 上就有一个[对比页面](https://en.wikipedia.org/wiki/Comparison_of_parser_generators)。

## 手写生成器

尽管解析器生成器非常好用：只要把文法倒进去，它就可以自动生成大量代码。但是有以下几个弊端：

1. 生成的是解析树而不是抽象语法树。这需要之后较多的人工工作来进行转换。
2. 报错和错误恢复可能比较复杂。
3. 如果生成器缺乏必需功能或者 bug，会造成很大的困扰。

在真实世界中，人们常常为了避免上述弊端而手写解析器。实践中，为了便于报错等，常常选择**自顶向下** (top-down) 解析器，或者是**递归下降** (recursive descent)，或者是 LL。

过去人们常常认为 top-down 解析无法处理左递归。实际上，存在一种名为 Pratt parser 的技术可以解决这个问题。它实际上是递归下降的一个简单变体，很容易理解，但又相当强大，非常适合处理表达式（递归、运算符有结合性）。这里给两个参考文章，供有兴趣的同学阅读。

1. [Simple Top-Down Parsing in Python](http://effbot.org/zone/simple-top-down-parsing.htm) (Python)
2. [Simple but Powerful Pratt Parsing](https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html) (Rust)
3. [Pratt Parsing Index](https://www.oilshell.org/blog/2017/03/31.html) (一篇调查文章)

## 解析器组合子

解析器组合子 (parser combinator) 是一种高阶函数，它可以把多个解析器组合成单个解析器。这是什么意思呢？又有什么应用价值呢？

首先需要定义这里所说的“解析器”是什么。在这里，解析器接受一段字符串，并返回解析的输出 **和** 剩下的字符串。所以，这里说的解析器实际上是一个函数。

举一个例子，假设存在词法分析器（它实际上也是一种解析器，但接受的文法是正则文法） `number` 和 `identifier`。

```
number("123abc") ==> (Some(123), "abc")  注意这里返回的是整数 123 而不是字符串
number("abc123") ==> (None, "abc123")    识别失败，因此 number 对应的输出是 None
identifier("abc123") ==> (Some("abc123"), "")  这里返回的是字符串 "abc123"
```

（`Some(x)` 表示解析成功，输出为 `x`；`None` 表示该解析器解析失败。）

假设这个语言是计算器的语言，支持用 `2x` 表示 `2*x`。所以 `factor` 可以是数字后接标识符。假如有一种方法，把 `number` 和 `identifier` 组合起来，岂不是很好？我们引入如下组合子：

1. `seq(p,q)`: 表示将输入按顺序经过 p 和 q，并输出两者的结果；如果其中某一步失败，则整个 `seq(p,q)` 也失败。
2. `or(p,q)`: 表示首先尝试 p，如果成功则返回结果，否则接着尝试 q，否则失败。

那么就可以定义
```
factor = or( 
  seq(number,identifier).map { |(a,b)| Expr.Mul(Expr.Const(a), Expr.Val(b)) },
  number.map(Expr.Const)
)
```

（上面的 `.map(...)` 用于将字符串或数字等数值转换成抽象语法树节点。可以与文法文件中的 action code 类比。）

根据上面说的，我们可以推测它的行为是：

```
factor("123") = (Some(Expr.Const(123)), "")
factor("2x") = (Some(Expr.Mul(Expr.Const(2), Expr.Val("x"))), "")
```

不难看出函数组合成大函数的过程，就是我们把小解析器组合成大解析器的过程，并且可以很自然地把自己想要的逻辑嵌入进去。更有趣的是，编译器是完全知道每个函数的类型的。

由此可见，解析器组合子是一种编程技巧而不是一种解析技术（解析技术是隐含在组合子的实现里的），使用这种技巧可以让代码模块化程度更高，并且在类型较强的语言中可以在编译时就捕获错误。此外，尽管代码是完全手写的，但代码却可以和使用解析器生成器一样干净整洁。感兴趣的同学请务必在自己喜欢的高级语言中尝试一番，或者亲自动手写一套组合子。

## 更多的解析技术

课本上介绍的解析技术非常实用，但并不是解析的全部。例如：

1. 可以处理二义文法和左递归的 [Earley parser](https://en.wikipedia.org/wiki/Earley_parser)。
2. 线性时间的 [Packrat parser](https://en.wikipedia.org/wiki/Parsing_expression_grammar)。
3. 使用动态规划思想设计的 $O(n^3 |G|)$ 时间的 [CYK 算法](https://en.wikipedia.org/wiki/CYK_algorithm)。
4. 哪怕是在解析已经被视为 solved problem 的 2020 年，还有诸如 [Pika parser](https://arxiv.org/abs/2005.06444) 之类的算法在不断被提出。

当然，这些算法知道名字就行了，没必要去花时间看。
