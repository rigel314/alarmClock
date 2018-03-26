const unsigned long crc_table[16] =
{
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

unsigned long crc32(char* addr, int len)
{
	unsigned long crc = ~0L;
	
	char* end = addr+len;
	
	for (; addr < end; addr++)
	{
		crc = crc_table[(crc ^ *addr) & 0x0f] ^ (crc >> 4);
		crc = crc_table[(crc ^ (*addr >> 4)) & 0x0f] ^ (crc >> 4);
		crc = ~crc;
	}
	return crc;
}
