class SimpleFileGenerator:
	def __init__(self, block_size=4096):
		self.block_size = block_size
		self.data = ''.join(['a' for i in xrange(block_size)])

	def generate(self, file_path, max_size):
		with open(file_path, "wb") as f:
			while max_size != 0:
				write_len = min(max_size, self.block_size)
				f.write(self.data)
				max_size -= write_len

gen = SimpleFileGenerator()
gen.generate("test_file", 1024 * 1024 * 512)
