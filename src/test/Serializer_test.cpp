#include "Serializer.h"

/**
 * Actually there is no need to use the new at all. Here are further possible improvements:
 * 1. make the MSG a return type of the deserialize function instead of an output parameter
 * 2. instead of typedef struct MSG{...}MSG; use struct MSG{...};, it's the same in C++
 * 3. use std::string or std::vector<byte> to hold the serialized data (and dispose of that #define PACKETSIZE altogether)
 *    and use it as a return value of the serialize function instead of the output parameter (or if you insist change the parameter to char(&data)[PACKETSIZE]).
 */

using namespace std;

int main() {
  Serializer serializer;
  uint8_t data[PACKETSIZE] = {0};
  i2c_msg_t msg;

  cout << "Original:" << endl;
  msg.img_id = 1;
  msg.img_chunkid = 9;
  msg.img_nchunks = 22;
  msg.img_w = 640;
  msg.img_h = 480;
  msg.type = 5;
  //msg.img_chunksize = 3;
  //msg.img_chunk[0] = 'H';
  //msg.img_chunk[1] = 'i';
  //msg.img_chunk[2] = '\0';
  msg.img_chunksize = 18;
  memcpy(&msg.img_chunk, (const uint8_t*)("Hello from HABPI!"), sizeof(msg.img_chunk));
  serializer.print(msg);

  cout << "Serialized:" << endl;
  serializer.serialize(&msg, data);
  for(int16_t i = 0; i < PACKETSIZE; i++) {
    cout << static_cast<int16_t>(data[i]) << " ";
  }
  cout << '\n' << endl;

  cout << "Deserialized:" << endl;
  i2c_msg_t temp;
  serializer.deserialize(data, &temp);
  serializer.print(temp);

  return 0;
}
