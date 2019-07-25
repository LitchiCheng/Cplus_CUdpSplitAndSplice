# Cplus_CUdpSplitAndSplice
Cplus_CUdpSplitAndSplice

# 注意事项
1. 单次拆包数据总大小不超过2 k。
2. 底层udp发包时，单包大小不超过304 bytes。(使用拆包后，每packet都为304 bytes)
3. 组包接收单个packet大小不超过304 bytes (底层udp接收限制。)
4. 在stm32中使用#define USING_IN_STM32 1
5. 在RBK中使用#define USING_IN_STM32 0
# 功能实现
1. CUdpSplitAndSplice udp_test;
2. udp_test.splitData(dd, 2000);//dd为拆包数据的地址，2000为数据长度。
3. udp_test.getSendPacketNum();//返回值为拆包数量。
4. udp_test.getSendData(send_index);//send_index为packet的序号，返回值为packet首地址，packet大小为304 bytes
5. udp_test.splicedata(recw,rec_size);//recw为接收数据地址，rec_size为接收数据长度。(只接受长度为304 bytes的数据包。)
6. udp_test.isSpliceFinshed();//返回值为组包是否完成。
7. udp_test.getStoreData();//返回值为组包后数据地址。
8. udp_test.getStoreDataSize()//返回值为组包后数据长度。
# 使用步骤
1. 创建实例。
2. splitData中传入拆包数据地址及拆包数据长度。
3. 发送时，循环数量为getSendPacketNum，发送getSendData中加入序号得到packet的数据地址。（底层发送时注意不可连续发送，适当异步延时处理。）
4. 接收时，循环接收，并在spliceData中传入接收到的数据地址及接收数据长度。
5. 判断isSpliceFinished为true时，表示组包完成且为完整的包。
6. getStoreData得到组包数据地址，getStoreDataSize得到组包数据长度。

# 测试代码
## 上位机代码
```
#include <iostream>
#include <string>
#include "C:\Users\hc\Desktop\boost_1_65_1\boost\asio.hpp"
#include "UdpSplitAndSplice.h"

namespace
{
	using namespace boost::asio;
	using namespace std;
}

int main()
{
	io_service io_s;
	ip::udp::socket udp_socket(io_s);
	ip::udp::endpoint       local_end(ip::address::from_string("192.168.192.33"), 45645);
	udp_socket.open(local_end.protocol());
	udp_socket.bind(local_end);
	uint8_t rec_data[4000] = { 0 };
	CUdpSplitAndSplice test;
	while (1)
	{
		ip::udp::endpoint remote_end;
		int len = udp_socket.receive_from(buffer(rec_data, 4000), remote_end);
		cout << "len is " << len << endl;
		if(len > 0)
			test.spliceData(rec_data, len);
		memset(rec_data, 0, 4000);
		if (test.isSpliceFinshed())
			break;
	}
	printf("\r\n");
	for (int i = 0; i < test.getStoreDataSize(); i++)
	{
		printf("0x%02x,",test.getStoreData()[i]);
	}
	return 0;
}
```

# 
# TODO
1. 检测未收到连续序号packet时应报错，并发起重发请求。
2. 收到一包packet，应与发送方握手，否则请求继续发送当前未收到的packet。

