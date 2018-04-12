#if !defined(_ROCKEY_API_INCLUDED_)
#define _ROCKEY_API_INCLUDED_

//From  https://bbs.pediy.com/thread-67191.htm

// ====================================================================

// 1. 查找计算机上连接的 Rockey2 加密锁设备
int WINAPI RY2_Find();
// 返回:
// 如果返回值小于 0，表示返回的是一个错误编码，具体含义请看下面的错误编码。
// 如果返回值为 0，表示没有任何 Rockey2 设备存在。
// 如果返回值大于 0，返回的是找到的 Rockey2 加密锁的个数。

// ====================================================================

// 2. 打开指定的加密锁
int WINAPI RY2_Open(int mode, DWORD uid, DWORD* hid);
// 输入:
// mode 是打开加密锁的方式
// mode = 0 表示的是打开第1个找到的 Rockey2，不理会 UID 和 HID
// mode > 0 表示的是按 UID 方式打开，这时候 mode 的值表示的是要打开锁的序号
//          譬如: uid=12345678, mode=2 表示的是希望打开 uid 等于12345678 的
//          第2把加密锁。
// mode = -1 表示的是按 HID 方式打开，要求 *hid 不能为 0
#define AUTO_MODE        0
#define HID_MODE        -1
// uid(UserID)
// 当按 UID 方式打开的时候，在这给出要打开加密锁的 UID，此 UID 就是用户调用
// RY2_GenUID 功能获得的用户 ID。
// hid
// 这个是加密锁的硬件 ID，这是1个输入/输出值，如果按 HID 方式打开加密锁的
// 时候，表示希望打开硬件ID = *hid 的加密锁。
// 不论以哪种方式打开加密锁，在成功打开加密锁以后会在这返回加密锁的硬件 ID
// 返回值:
// 大于等于0                表示成功，返回的就是打开的加密锁的句柄
// 小于0                        返回的是一个错误编码，具体含义请看下面的错误编码部分。

// ====================================================================

// 3. 关闭指定的加密锁
void WINAPI RY2_Close(int handle);
// 输入:
// handle 设备的句柄，跟 RY2_Open 命令返回的 handle 一致。
// 返回:
// 返回的是一个错误编码，具体含义请看下面的错误编码部分。

// ====================================================================

// 4. 生成用户 ID
int WINAPI RY2_GenUID(int handle, DWORD* uid, char* seed, int isProtect);
// 输入:
// handle 设备的句柄，跟 RY2_Open 命令返回的 handle 一致。
// uid 输出参数，生成的用户 ID 从此参数返回
// seed 用户给出的用于生成用户 ID 的种子，这是一个最大长度可以是 64 个字节的字符串
// isProtect 用来指定此 Rockey2 是否可写，0 是可写，1 是不可写
#define ROCKEY2_DISABLE_WRITE_PROTECT        0
#define ROCKEY2_ENSABLE_WRITE_PROTECT        1
// 返回:
// 返回的是一个错误编码，具体含义请看下面的错误编码部分。

// ====================================================================

// 5. 读取加密锁内容
int WINAPI RY2_Read(int handle, int block_index, char* buffer512);
// 输入:
// handle 设备的句柄，跟 RY2_Open 命令返回的 handle 一致。
// block_index 块索引，指明要读取的是哪1个块，取值为(0-4)
// buffer512 读出块的缓冲区，因为每个块的长度固定为 512 个字节，所以这个
// buffer 的大小至少是 512 个字节
// 返回:
// 返回的是一个错误编码，具体含义请看下面的错误编码部分。

// ====================================================================

// 6. 写入加密锁内容
int WINAPI RY2_Write(int handle, int block_index, char* buffer512);
// 输入:
// handle 设备的句柄，跟 RY2_Open 命令返回的 handle 一致。
// block_index 块索引，指明要写入的是哪1个块，取值为(0-4)
// buffer512 写入块的缓冲区，因为每个块的长度固定为 512 个字节，所以这个
// buffer 的大小至少是 512 个字节
// 返回:
// 返回的是一个错误编码，具体含义请看下面的错误编码部分。

// 7. 进行加密变换
int WINAPI RY2_Transform(int handle, int len, BYTE *data);
// 输入:
// handle 设备的句柄，跟 RY2_Open 命令返回的 handle 一致。
// len 数据块的长度，最大为64
// data 输入输出缓冲区,这是一个最大长度可以是 64 个字节的字符串
// 返回:
// 返回的是一个错误编码，具体含义请看下面的错误编码部分。
// 错误编码 ===========================================================

// 成功，没有错误
#define RY2ERR_SUCCESS                    0

// 没有找到给定要求的设备(参数错误)
#define RY2ERR_NO_SUCH_DEVICE             0xA0100001

// 在调用此功能前需要先调用 RY2_Open 打开设备(操作错误)
#define RY2ERR_NOT_OPENED_DEVICE          0xA0100002

// 给出的 UID 错误(参数错误)
#define RY2ERR_WRONG_UID                  0xA0100003

// 读写操作给出的块索引错误(参数错误)
#define RY2ERR_WRONG_INDEX                0xA0100004

// 调用 GenUID 功能的时候，给出的 seed 字符串长度超过了 64 个字节(参数错误)
#define RY2ERR_TOO_LONG_SEED              0xA0100005

// 试图改写已经写保护的硬件(操作错误)
#define RY2ERR_WRITE_PROTECT              0xA0100006

// 打开设备错(Windows 错误)
#define RY2ERR_OPEN_DEVICE                0xA0100007

// 读记录错(Windows 错误)
#define RY2ERR_READ_REPORT                0xA0100008

// 写记录错(Windows 错误)
#define RY2ERR_WRITE_REPORT               0xA0100009

// 内部错误(Windows 错误)
#define RY2ERR_SETUP_DI_GET_DEVICE_INTERFACE_DETAIL 0xA010000A

// 内部错误(Windows 错误)
#define RY2ERR_GET_ATTRIBUTES             0xA010000B

// 内部错误(Windows 错误)
#define RY2ERR_GET_PREPARSED_DATA         0xA010000C

// 内部错误(Windows 错误)
#define RY2ERR_GETCAPS                    0xA010000D

// 内部错误(Windows 错误)
#define RY2ERR_FREE_PREPARSED_DATA        0xA010000E

// 内部错误(Windows 错误)
#define RY2ERR_FLUSH_QUEUE                0xA010000F

// 内部错误(Windows 错误)
#define RY2ERR_SETUP_DI_CLASS_DEVS        0xA0100010

// 内部错误(Windows 错误)
#define RY2ERR_GET_SERIAL                 0xA0100011

// 内部错误(Windows 错误)
#define RY2ERR_GET_PRODUCT_STRING         0xA0100012

// 内部错误
#define RY2ERR_TOO_LONG_DEVICE_DETAIL     0xA0100013

// 未知的设备(硬件错误)
#define RY2ERR_UNKNOWN_DEVICE             0xA0100020

// 操作验证错误(硬件错误)
#define RY2ERR_VERIFY                     0xA0100021

// 未知错误(硬件错误)
#define RY2ERR_UNKNOWN_ERROR              0xA010FFFF 


#endif //_ROCKEY_API_INCLUDED_

