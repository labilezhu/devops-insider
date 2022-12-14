# 从《Why I Left Facebook》扯到蘇東坡《卜算子》


![](why-i-left-facebook.assets/logo.jpg)


前段时间，由于要研究一个 [TCP 接收缓冲区大小配置的问题](https://blog.mygraphql.com/zh/notes/low-tec/network/tcp-mem/)，搜索到了一编 Blog： [A TCP Timeout Investigation](https://hechao.li/2022/09/30/a-tcp-timeout-investigation/)。 感觉 Blog 主是个现世小众的技术较真之人。 于是浏览了他的其它文章，看到一编： [Why I Left Facebook](https://hechao.li/2022/06/24/why-i-left-facebook/)。感触良多，故翻译之。

> <mark>⚠ 警告：本文无关技术</mark>

## [Why I Left Facebook] 译文

> 以下引用自：[Why I Left Facebook](https://hechao.li/2022/06/24/why-i-left-facebook/)。作为局外人，译者无法验证原作者的说法的正确性，所以，本译文只供参考。作者当然有一定的个人感情在里面了。在自认 EQ 高的，精通各种成功学理论的人眼中，`快意恩仇` 是个 EQ 低的表现。但谁能否认，社会与技术的很多变化，不是由一次次由 `快意恩仇` 的人去触发的？ 如果我们一直不去直面黑暗面，忌讳黑暗面，那么黑暗面迟早会吞噬所有。

> <mark>⚠ 警告：以下只是引文与翻译，内容与译者无直接相关性，请看官不要对号入座，上岗上线。</mark>

### 一个过期的帖子
我不得不说这篇文章姗姗来迟。虽然我离开 Facebook 才一个月，但这篇文章的草稿已经存在了将近 2 年。这些年来，我多次想过离开。每当我这样做时，我发誓要写下每一个让我离开的原因，并在我离开之前在内部发布。具有讽刺意味的是，当我真正离开时，我没有留下一个字。事实上，这是我第一次在这个技术博客上谈论个人内容的一个重要原因——我不想浪费草稿 :-) 。

请注意，我仍然会使用 “`Facebook`” 而不是 “`Meta`”，因为它在草稿中使用过，我懒得更改它。也因为我不喜欢这个新名字，这另一个话题，我不打算展开来说了。

### 原因不是根本原因

我没有像我发誓那样发帖，因为直接导致我离开的原因并不是我离开的主要原因(原文：what caused me to leave is not why I left)。这听起来很奇怪。让我解释。促使我找到下一份工作的直接原因是取消了我最喜欢的项目以及接下来的重组。但这不是根本原因。因为 Facebook 的团队转换很简单，所以我很容易找到另一个团队。然而我还是选择了离开公司而不是换团队。原因如下。

### 社交软件不适合我

在我的脑海中，有两种社交媒体：
- 一种是你与现实生活中认识的人（例如 `Facebook` 和 `Instagram` ）炫耀你的自命不凡生活
- 另一种是你向陌生人宣战，因为他们喜欢披萨上的菠萝（例如 `Twitter` ）

当我在这篇文章中说“社交媒体”时，我指的是前者。

我从未使用过 `Facebook` 或 `Instagram` 。只有一次，我不得不在入职 Facebook 前一天注册了我的 Facebook 帐户，只是因为公司要求提供。我用过的唯一社交媒体是微信朋友圈。<mark>当我意识到：我为向 `“朋友”` 展示最好的自己时，让自己的内在变成一个最糟糕的自己时，我就停止了使用它。(原文：I stopped using it when I realized that I became the worst of myself when I tried to present the best of myself to “friends”)</mark>。那时，我变得痴迷于喜欢和评论。当我出去的时候，我不能停止思考要发布哪些照片才能获得最多的喜欢。我想留下这样的印象，即事实上的千载难逢的郊游是我的日常生活（原文：I wanted to leave the impression that the de facto once-in-a-blue-moon outing was my daily life.）。

因此，<mark>我对公司的核心产品也毫不在意（原文：I couldn’t care less about the company’s core products.）</mark>。我专注于技术是因为它本身很酷，而不是为了支持业务。结果，我对自己的工作并不感到自豪。说句公道话，我曾为曾经参与过一个非常酷的项目而感到自豪，但不幸的是它被取消了 T_T 。正是那时，我决定为另一家我实际使用和关心其产品的公司工作。

### 社交软件文化

老实说，当我意识到我加入了一家社交媒体公司的第一天，我怀疑我加入 Facebook 的决定。这听起来真的很愚蠢。在我决定加入之前我不应该知道吗？就像我说的，我不在乎产品。然而，直到在 Facebook 的第一天，我才意识到该公司的产品也可能影响其文化（或者可能是相反的方式？）。当我开始觉得评价他人，就像现实生活中的社交媒体互动时，我想到了这个想法，因为我们被要求在所有时候竖起大拇指（即赞赏）。我称之为“社交媒体文化”。

我上面提到我不喜欢社交媒体，主要是因为社交媒体泡沫中的一切都是那么闪亮和美丽，不真实的感觉让我感到厌恶。它就像甜味剂 —— 它肯定很甜，但尝起来很假。这就是我在 Facebook 最初几个月的感受。每个人都很好，但也太好了(原文：Everyone is nice, but too nice)。

例如，我发现人们高度评价每件事和每个人。在`《老友记》`中，`菲比`曾经和一个叫`帕克`的人约会过，他一直在赞美一切。当`菲比`抱怨时，他说，“那只是因为我是一个积极的人。” 然后 `菲比` 喊道：“不！我是一个积极的人。在百忧解(Prozac)上，你就像圣诞老人一样！在迪士尼乐园！Getting laid！”

![](why-i-left-facebook.assets/friends_parker.png)

当然，`帕克`只是一个为了戏剧效果而被夸大的角色，但我觉得在最初的几个月里就像和`帕克`一起工作。 例如，当我抱怨某事或某人时，人们会说：“反馈是一种礼物。 如果你不喜欢它，你应该给出反馈并帮助他们改进。”  好吧，<mark>为什么我们不能承认有些人只是混蛋，不会改变？(why can’t we admit that some people are just jerks and won’t change?)</mark> 公平地说，我确实给出了反馈，甚至几次受到打击。 一个极端的例子是，如果我对一个人的反馈是 “脾气暴躁，不愿意接受反馈” ，你认为他们会神奇地接受这种反馈吗？

至少我可以说我不是圣人，我会情不自禁地抱怨事情，抱怨混蛋。 我甚至通过这样做在 Facebook 结交了我的第一个朋友。 我仍然很高兴前几天我开始咆哮，他们开始回应。 最后我们都说 “哦，谢天谢地，你很正常。”


### 聪明的混蛋(briliant jerks)
当人们问我喜欢在 Facebook 工作的哪些方面时，我总是提到的一件事就是与才华横溢的人一起工作。我必须说，我与许多我在 Facebook 认识的最敏锐的人一起工作，我真的很感激这个机会。然而，不幸的是，我也遇到了比其他地方更多的 “`聪明的混蛋(briliant jerks)`”。我想每个人都或多或少地与一个`聪明的混蛋`一起工作，所以你知道我在说什么样的人。

与`聪明的混蛋`一起工作的问题在于，<mark>在技术讨论中，他们的目标不是通过健康的辩论和讨论做出正确的决定，而是语言取巧他人(👨 译者注：语言话术上取巧，或让对手因情绪失控而出现失误，然后攻击其失误。因为你失误了，所以你的论点不值一提，我的才是对的)</mark>。作为工程师，我们应该知道大多数技术问题没有正确或错误的答案，因为这都是权衡的取舍。但显然，对于`聪明的混蛋`来说，总是有正确的答案 —— 他们的答案。

一个 `聪明的混蛋` 的有毒行为可能会让其他人，尤其是新来的人非常沮丧。我第一次和一个`聪明的混蛋`一起工作时，它引发了`冒名顶替综合症(imposter syndrome)`，让我觉得我太愚蠢了，不能和他们一起工作。我花了一些时间来克服它。现在，如果我必须与一个试图比我聪明的人一起工作，我只想说，<mark>“如果你真的那么聪明，那你为什么要困在一家雇用愚蠢如我的愚蠢的公司？”(If you are really that smart, then why are you stucking in a stupid company that hired stupid me?)</mark>

最让我恼火的不是那些`聪明的混蛋`本身（毕竟，我可以和朋友抱怨他们），而是他们的行为经常被管理层忽视。在 Facebook，我从来没有见过一个`聪明的混蛋`被作为一个混蛋而受到惩罚，但只看到他们因为聪明而受到奖励。他们经常被介绍为 “团队中最好的工程师”。

> 译注：  
>
> 比小数 “混蛋” 更让人绝望的是大多数人对 “混蛋” 行为的 “沉默”，有时甚至出于某种原因 “助纣为虐”。判断事情的标准由 “朴素价值” 变成 “利益得失”。而为了向其内心仅存的 “朴素价值” 作出解释，想出了几个办法：“情商”、“下大棋”、“成熟的表现”、“人生智慧”。`精致利己-lism` 可以在 `Org` 的上升期推动发展，但在平稳期与收缩期，这多数情况下是负作用。哪怕是在 `Org` 的上升期，这种情况也对创新有打击。因为创新的人很少同时具有 “混蛋” 的性格 “优点”，一般都只能被  “混蛋”  混蛋。



### PSC - 可悲的挣扎周期(Pathetic Struggling Cycle)

如果你有朋友在 Facebook 工作，你一定听说过 PSC 这个词。甚至一年听两次。 PSC 是* Facebook 每年两次的绩效评估。在 PSC 季节（是的，这个季节可以持续 2 个月），人们会撰写自我评价和同行评价。在循环结束时，您将获得与您的奖金相关的评级。这是一个从“不符合（期望）”到“重新定义（期望）”的 7 个等级的评级。

我也怀疑 “社交媒体文化” 源于 “PSC文化” 。也许是因为同事评议可能会影响一个人的评级，从而影响奖金（金钱），人们试图对彼此非常友善，希望其他人在同事评议中谈论他们（请注意，同事评议也与经理分享）。至于一些混蛋，如果你赞许他们，他们肯定会说你短处。尽管系统中有建设性反馈部分，但在我收到的所有同事评议中，我从未得到“真正的”建设性反馈。他们要么是糖衣，要么根本没有建设性的反馈 —— “期待下半场和你一起做项目 X” —— 这是什么反馈？在我的第一个 PSC 中，我确实向人们提供了一些建设性的反馈。但是当我收到别人的所有好话后，我感觉很糟糕，不再这样做了。

但是，与它造成的最大问题 —— `面向 PSC 的工程文化(PSC-oriented engineering)`相比，糖衣反馈甚至不是问题。 PSC中最重要的因素是“`影响（impact）`”。但是你如何定义 “`影响`”？ 没人知道。<mark>多年来，具有高知名度的新项目和闪亮项目比修复错误更具“影响”，这已成为一条不成文的规则(Over the years, it has become an unspoken rule that new and shiny projects with high visibility has more “impact” than say, fixing bugs)</mark>。此外，当人们说：“在 PSC 期间，经理不会将您与其他人进行比较时，你只是与预期进行比较。”，我不相信他们。而且我不相信一个在遗留系统中修复了 10 个 Bug 的人会比一个在生产环境中推出 1 项新服务的人获得更多的分数，无论它是否被使用。

因此，想要获得良好评级的人必须找到新的工作领域，并尝试在下一次 PSC（最多 6 个月）之前展示一些结果。有时，当我在代码中看到一些`空架子（烂代码）(crap)`，并询问为什么要这样设计时，人们可以花一页时间解释“历史原因”。但在我经历了几次做出某些决定的“`历史`”之后，我终于意识到，<mark>真正的原因只不过是在 PSC 之前完成这个废话并收集学分。当然，人们不会大声说出来(the real reason is nothing but to finish this crap before PSC and collect credits)</mark>。但是，对于持续数年的非紧急问题，您还能如何解释所有这些“短期”解决方案？

我主要从事系统和基础设施方面的工作。众所周知，许多系统级项目很容易需要多年的努力。要在 6 个月内显示一些结果，你别无选择，只能把事情搞砸。为什么我需要花一个月的时间来编写设计文档，而我可以在 2 周内写出“刚刚好”的东西，在一天内将其投入生产，并在我的 PSC 审查中用一段话来吹嘘它？因此，几十年来在生产中运行设计不佳的系统并不常见。显然，不值得修复它们。最好写一个新的烂系统来代替旧的。记住？影响力(Impact)！

我必须说，在做出技术决策时，<mark>我已经厌倦了争论长期而不是短期的解决方案</mark>。看到聪明的人因为愚蠢的系统做出愚蠢的决定，真是令人难过。我相信最初 PSC 的目的是提高人们的表现。但不幸的是，<mark>该系统(PSC)成为了目的本身(the system became the purpose itself.)</mark>。

这个说法，或者有点点过时，因为他们从 2022 年开始将 PSC 改为每年两次。这会改变什么吗？我不知道。

### 升职 … 或 走人
与 PSC 系统类似，练级系统是另一种具有明确规则的游戏，您必须玩。每个级别的期望都不同，并记录在 wiki 中。简而言之，E3 被期望在任务上工作；预计 E4 将致力于功能； E5 预计拥有一个项目； E6 预计将领导多个项目； E7 预计会做出跨组织的贡献；等等

Facebook 有一个“升级或退出”政策，这意味着如果你被聘为初级工程师（E3 或 E4），那么你必须在 X 个月（我不记得确切的数字）内晋升为 E5，否则你就是出去。为了让它更有趣，如果你没有在 Y 中晋升，其中 Y < X，月，那么你进入一个黄色区域。如果你在 Z 中没有晋升，其中 Y < Z < X，月，那么你进入一个红色区域。一旦你进入红色区域，即使你是 E4，你也会被评估为 E5。如果你没有达到 E5 的预期，你就出局了。

我被聘为 E4，直到黄色区域结束时才能达到 E5。在我在 Facebook 的头两年，滴答作响的时钟让我的生活充满压力。在某一时刻，我真的认为我所做的工作与其他 E5 没有什么不同，但我就是无法升职。当我问为什么，他们会说，“是的，你做得很好。但在 Facebook，我们会进行`末位晋升（trailing promotion）`，这意味着你必须证明你可以始终如一地在新的水平上表现”。我的经理告诉我，“始终如一”意味着至少 2 个一半（1 个一半 = 6 个月）。换句话说，当我晋升到 E5 时，我肯定已经从事 E5 工作至少一年了。

这听起来不公平，是吗？我以 E4 的身份获得报酬，但必须至少做一年的 E5 工作才能证明我可以成为 E5。我不得不努力工作，以免被踢出去。剥削初级工程师的好方法！如果我提出金钱问题，他们会说：“你还年轻。不要过分关注金钱。专注于增长。”每当我听到这样的废话时，我的眼睛就停不下来。

My favorite scene in Stranger Things is when Dustin was trying to trick Erica to perform a task that only she could do, he said “This is for your country. Don’t you love your country?” Erica said, “You can’t spell America without Erica … You know what I love most about this country? Capitalism. Do you know what capitalism is? It means this is a free market system, which means people get paid for their services, depending on how valuable their contributions are.”


好吧，我只是想在这个 free market 上玩一场公平的游戏，突然间我变得“太注重金钱了”。 然后它让我别无选择，只能找到另一个更重视我的贡献的地方。


### 令人失望的 Leadership

自从我在公司工作以来，人们一直在说 Facebook 有一种“`自下而上`”的文化，这意味着项目和决策主要由 IC 驱动。直到有一天我最喜欢的项目被`自上而下`的决定取消了。从那时起，我注意到在组织和公司层面做出了许多自上而下的决策，尽管 IC 对他们的反馈强烈，甚至在没有 IC 参与的情况下做出。更重要的是，Leadership 层似乎真的在捍卫这些决定并决心执行它们。

一个不寻常的例子是，当公司开始削减福利，将晚餐从下午 6 点推迟到 6:30 以及取消外卖盒时，公司的 CTO 以好斗的口吻与许多员工为这一决定辩护。当我看到对每个愤怒个人评论的回复时，我真的很惊讶。我就像“伙计，你是公司的首席技术官。你没有更好的事情要做吗？”

事实上，让人生气的不仅是`自上而下`的决定，或者他们没有吃晚饭，而是 Leadership 发出的信息 —— “晚饭只给加班的人。如果你想吃晚饭，那就工作到很晚。”老实说，与其他地方相比，Facebook 的 `工作与生活平衡(work-life balance(WLB))` 总体上已经很糟糕，部分原因是 PSC 文化。很长一段时间，我没有 `WLB` ，只有 `WWWWWWWWWWWWWWLB` 。然而，糟糕的 `WLB` 是一回事，Leadership 层鼓励的糟糕的 `WLB` 是另一回事。晚餐时间之战，我不是第一次从 Leadership 那里听到这样的信息。大约在同一时期，扎克伯格也发表了评论，基本上意味着 996 比每周工作 4 天要好。

### 最后几句话
虽然我已经离开了公司，但我仍然有点担心它的有毒文化，因为它似乎具有传染性。 与我交谈过的不止一个人，包括那些担任管理职位的人，都非常尊重 Facebook 文化，甚至试图将其带到他们自己的公司。 在我看来，让 Facebook 变得伟大（嗯，就规模和金钱而言）的不是蹩脚的文化。而是不管系统有缺陷，才华横溢的人都能产生真正的影响。 他们不应该得到公司现有的对待。

## 译者结语

这篇文章，有两方面让我有一定的共情：

- 网络社交文化的副作用
- 人类社会永远客观存在的逆向淘汰

一千年前，蘇東坡在乌台诗案后，被贬黄州时，作《卜算子》：

>  卜|  　|  驚|  缺|  
>  算|  　|  起|  月|  
>  子|  　|  卻|  掛|  
>  　|  　|  回|  疏|  
>  ┃|  　|  頭|  桐|  
>  　|  　|  ，|  ，|  
>  黃|  　|  有|  漏|  
>  州|  　|  恨|  斷|  
>  定|  　|  無|  人|  
>  慧|  　|  人|  初|  
>  院|  　|  省|  靜|  
>  寓|  　|  。|  。|  
>  居|  　|  揀|  時|  
>  作|  　|  盡|  見|  
>  　|  　|  寒|  幽|  
>  ┃|  　|  枝|  人|  
>  ┃|  　|  不|  獨|  
>  　|  　|  肯|  往|  
>  蘇|  　|  棲|  來|  
>  軾|  　|  ，|  ，|  
>  　|  　|  寂|  縹|  
>  　|  　|  寞|  緲|  
>  　|  　|  沙|  孤|  
>  　|  　|  洲|  鴻|  
>  　|  　|  冷|  影|  



有个心理学家说过：

> 如果我们时常回忆过往的坏事情，就像一个人时常在粪坑上打滚，是很难得到快乐的。

或者正能量不是去改变什么，而是认识到不能改变什么，并尝试不沉迷其中，用更多的生命能量去完成那些值得的事情。

> 本文摘自我在整理中的[《Mark’s DevOps 雜碎》](https://devops-insider.mygraphql.com/zh_CN/latest/index.html) 一书中的 [从《Why I Left Facebook》扯到蘇東坡《卜算子》](https://devops-insider.mygraphql.com/zh_CN/latest/wu/career/jerks/why-i-left-facebook/why-i-left-facebook.html) 一节。
