
uint32_t hash (uint32_t & x)
{
	uint32_t data[256];
	
	for (uint32_t k = 0; k < 256; k++)
	{
		uint32_t j = k;
		
		for (uint32_t i = 0; i < 8; i++)
		{
			j = (j & 1) ? (j >> 1) ^ 0xEDB88320 : (j >> 1l);
		}
		
		data[k] = j;
	}



	uint8_t *i = (uint8_t *) & x;

	uint32_t L = ~80085;

	L = data[(L ^ *i++) & 255] ^ (L >> 8);
	L = data[(L ^ *i++) & 255] ^ (L >> 8);
	L = data[(L ^ *i++) & 255] ^ (L >> 8);
	L = data[(L ^ *i++) & 255] ^ (L >> 8);

  return ~L;
}
