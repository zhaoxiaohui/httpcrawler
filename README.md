**Usage:**
----------
输入 如： ./siteanalyzer http://xxx.xxx.xxx.xxx/techqq/index.html check.dat [80]

*Args:*
- 初始入口网址
- 执行完爬行之后，需要检查的URL，每行一条，最终将检查结果存放在checkresult.dat文件
- 网站端口，默认为8080

**程序目标**
----------
- 抓取网站链接，计算每个连接的PR(pagerank)，并建立倒排索引
- 生成PR最高的前十URL
- 指定查询URL，输出PR值等信息

**程序设计**
----------
本程序的结构分为三块：主运行模块、webgrap模块、urlqueue模块、urlhash模块、network模块、http_client模块、link_parse模块、crawler_http模块。八个模块完成的具体功能如表1下：

![结构设计](http://i.imgur.com/nhaCQpw.png)
- 主运行模块
  - 负责对各个模块的调用，等待网页分析结束，并通知分析线程销毁自己，之后通知webgrap生成各个输出结果。
- webgrap模块
  - 记录网页的结点和链接关系，并能有效进行插入标记等等的操作，最后能够根据链接情况生成入度累计分表、计算Pagerank值等等功能。
- urlqueue模块
  - 记录当前分析出的网页链接，以便用于之后的链接分析。
- urlhash模块
  - 此模块主要用来对当前分析的URL进行hash计算，便于数据的存储访问。
- network模块
  - 负责socket链接的打开和关闭功能。
- http_client模块
  - 主要用来对打开的链接发送请求，取得返回内容并能分析返回结果。
- link_parser模块
  - 根据当前取得的网页内容，分析出对于本次实验有效的网页链接。
- crawler_http模块
  - 此模块主要通过线程池对urlqueue中的链接进行并行的访问，并存储link_parser的分析结果。实际上就是对其他模块的多线程控制。

**注意**
----------
运行完之后系统会打印 calculating done字样，并在同文件夹下 生成indcdf.dat top10.dat checkresult.dat文件


程序运行时间估计在20分钟左右（网络速度在10M/S左右，并且机器性能相对较好）
