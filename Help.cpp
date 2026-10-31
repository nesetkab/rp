#include <string>
int countChars(const std::string &str) {
  // one of the block characters is 3 bytes
  // so .size() returns something larger than the actual length
  // this is a workaround c:
  int length = 0;
  for (unsigned char c : str) {
    if ((c & 0xC0) != 0x80) {
      length++;
    }
  }
  return length;
}
