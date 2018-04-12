
真假Rockey2完整源码

作者：[achillis](https://bbs.pediy.com/thread-225405.htm)

---

程序介绍：

TrueRockey2 顾名思议，这是真正的Rockey2.dll的源码，可以对真实的狗进行查找、读、写等操作

FakeRockey2 这个是假的Rockey2.dll，只提供了相应的接口，用来模拟Rockey2狗

Rockey2Test 这个是个简单的Rockey2的测试程序

---

加密狗破解：

直接用我们的假Rockey2.dll替换掉这个动态库是最简单的破解方法。

---

一、了解加密狗流程

二、使用hid.lib于枚举USB设备，全称Human Interface Device (HID)。