/** 
 * Sample program for compression module
 **/
 
#include "../Src/Compress/Compress.hpp"
#include <fstream>
#include <iterator>
int main() {
	using namespace Kelpa::Compress;
	using namespace Kelpa::Utility;
	
	std::fstream ifs("./input.txt", std::ios_base::binary | std::ios_base::in);	
	auto sheet = Huffman::Encoder(std::istream_iterator<unsigned char>(ifs), std::istream_iterator<unsigned char>(ifs)).Encode();
	
	std::fstream ofs("./output.bin", std::ios_base::binary | std::ios_base::out);
	ifs.seekg(0, std::ios_base::beg);
	Huffman::Writer(
		std::istream_iterator<unsigned char>(ifs), 
		std::istream_iterator<unsigned char>(ifs), 
		std::ostream_iterator<char>(ofs)
	).Write(sheet);
	
	ifs.close();
	ofs.close();
	ifs.open("output.bin", std::ios_base::binary | std::ios_base::in);
	ofs.open("output.txt", std::ios_base::binary | std::ios_base::out);
	
	Huffman::Reader(
		std::istream_iterator<unsigned char>(ifs), 
		std::istream_iterator<unsigned char>(ifs), 
		std::ostream_iterator<char>(ofs)
	).Read();
	
	return 0;
}
