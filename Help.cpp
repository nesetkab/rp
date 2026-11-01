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

int bytesForChars(const std::string &str, int chars) {
  int count = 0;
  for (std::size_t i = 0; i < str.size(); i++) {
    if ((static_cast<unsigned char>(str[i]) & 0xC0) != 0x80) {
      if (count == chars) {
        return static_cast<int>(i);
      }
      count++;
    }
  }
  return static_cast<int>(str.size());
}
