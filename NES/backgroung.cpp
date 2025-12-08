#include "backgroung.h"
#include"nes_pal.h"
#include<assert.h>

uint8_t NametableData::read_tile_no(int x, int y, uint8_t table) const
{
	assert(table < 2);
	y %= 30;
	x %= 32;
	auto addr = int(table * 1024) + (y * 32) + x;
	return table_data[addr];
}

uint8_t NametableData::read_tile_attrib(int x, int y, uint8_t table) const
{
	assert(table < 2);
	y %= 30;
	x %= 32;
	auto addr = int((table * 1024) + 960) + ((y / 4) * 8) + (x / 4);
	return table_data[addr];
}

void NametableData::write(uint16_t address, uint8_t data) {
	table_data[address & 0x7FF] = data;
}

uint8_t NametableData::read(uint16_t address) const {
	return table_data[address & 0x7FF];
}

NametableInterface::NametableInterface(NametableData* data, ReadWriteable* rw)
{
	enabled = false;
	show_leftmost = false;
	coarse_x = 0;
	fine_x = 0;
	coarse_y = 0;
	fine_y = 0;
	_coarse_x = 0;
	_fine_x = 0;
	_coarse_y = 0;
	_fine_y = 0;
	table = 0;
	_table = 0;
	assert(data != nullptr);
	this->data = data;
	this->rw = rw;
	tbl = BackgroundTable::TABLE_0000;
}

void NametableInterface::draw(
	uint8_t(*palette)[4][4], 
	int scanline, 
	Color(*pixels)[256],
	uint8_t(*priority)[256])
{
	for (int i = 0; i < 256; ++i) {
		(*pixels)[i] = NES_PAL[(*palette)[0][0]];
	}
}

uint8_t NametableInterface::calc_attrib_shift(int x, int y)
{
	return uint8_t((y & 0x02) << 1) | uint8_t(x & 0x02);
}

uint16_t NametableInterface::get_pattern(int tile_no) const
{
	uint16_t pattern = 0;
	uint8_t bit_mask = 0x80;
	int offset = 16 * tile_no;
	auto base_addr = uint16_t(tbl);
	uint8_t plane0 = rw->read(base_addr + offset + fine_y);
	uint8_t plane1 = rw->read(base_addr + offset + 8 + fine_y);
	for (int i = 7; i >= 0; --i) {
		pattern |= (uint16_t(bit_mask & plane0) << i);
		pattern |= (uint16_t(bit_mask & plane1) << (i + 1));
		bit_mask >>= 1;
	}
	return pattern;
}

void NametableInterface::next_x() {
	fine_x += 1;
	if (fine_x == 8) {
		coarse_x += 1;
		fine_x = 0;
	}
	if (coarse_x > 31) {
		table ^= 1;
		coarse_x = 0;
	}	
}

void NametableInterface::next_y() {
	fine_y += 1;
	if (fine_y == 8) {
		coarse_y += 1;
		fine_y = 0;
	}
	if (coarse_y > 29) {
		table ^= 1;
		coarse_y = 0;
	}
}

VerticalInterface::VerticalInterface(NametableData* data, ReadWriteable* rw)
	:NametableInterface(data, rw)
{
}

uint8_t VerticalInterface::read(uint16_t address)
{
	int t;
	normalize_address(&address, &t);
	return data->read((1024 * t) + address);
}

void VerticalInterface::write(uint16_t address, uint8_t value)
{
	int t;
	normalize_address(&address, &t);
	data->write((1024 * t) + address, value);
}

void VerticalInterface::set_scroll(uint16_t reg, uint8_t fine_x)
{
	_coarse_x = uint8_t(reg & 0x001F);
	_fine_x = fine_x;
}

void VerticalInterface::draw(
	uint8_t(*palette)[4][4], 
	int scanline, 
	Color(*pixels)[256],
	uint8_t(*priority)[256])
{
	if (!enabled) {
		NametableInterface::draw(palette, scanline, pixels, priority);
		return;
	}

	coarse_y = scanline / 8;
	fine_y = scanline % 8;

	for (int curr_pixel = 0; curr_pixel < 256; ++curr_pixel) {
		auto tile_no = data->read_tile_no(coarse_x, coarse_y, table);
		auto attrib_data = data->read_tile_attrib(coarse_x, coarse_y, table);
		uint8_t attrib_shift = calc_attrib_shift(coarse_x, coarse_y);
		auto pattern = get_pattern(tile_no);

		uint8_t temp = attrib_data >> attrib_shift;
		uint8_t attrib = temp & 0x03;
		uint8_t backdrop = (*palette)[0][0];

		uint8_t pal_index = (pattern >> (2 * (8 - fine_x - 1))) & 0x03;//Pixel value from pattern table
		uint8_t pal = (*palette)[attrib][pal_index];
		Color pixel = NES_PAL[pal];

		if (pal_index == 0) {
			(*pixels)[curr_pixel] = NES_PAL[backdrop];
		}
		else {
			(*pixels)[curr_pixel] = pixel;
			(*priority)[curr_pixel] = 1;
		}
		next_x();
	}
}

void VerticalInterface::set_base_address(uint8_t opt)
{
	switch (opt) {
	case 0: case 2:
		_table = 0;
		break;
	case 1: case 3:
		_table = 1;
		break;
	default:
		assert(false);
	}
}

void VerticalInterface::begin()
{
	coarse_x = _coarse_x;
	fine_x = _fine_x;
	table = _table;
}

void VerticalInterface::reset(bool flag)
{
	coarse_y = fine_y = 0;
}

void VerticalInterface::normalize_address(uint16_t* address, int* table) {
	
	if ((*address >= 0x2000 && *address < 0x2400) ||
		(*address >= 0x2800 && *address < 0x2C00)) {
		*table = 0;
	}
	else if ((*address >= 0x2400 && *address < 0x2800) ||
		     (*address >= 0x2C00 && *address < 0x3000)) {
		*table = 1;
	}
	else {
		assert(false);
	}
	*address &= 0x3FF;
}

HorizontalInterface::HorizontalInterface(NametableData* data, ReadWriteable* rw)
	:NametableInterface(data, rw)
{
}

uint8_t HorizontalInterface::read(uint16_t address)
{
	int t;
	normalize_address(&address, &t);
	return data->read((1024 * t) + address);
}

void HorizontalInterface::write(uint16_t address, uint8_t value)
{
	int t;
	normalize_address(&address, &t);
	data->write((1024 * t) + address, value);
}

void HorizontalInterface::set_scroll(uint16_t reg, uint8_t fine_x)
{
	_coarse_y = uint8_t((reg & 0x03E0) >> 5);
	_fine_y = uint8_t((reg & 0x7000) >> 12);
}

void HorizontalInterface::draw(uint8_t(*palette)[4][4], int scanline, Color(*pixels)[256], uint8_t(*priority)[256])
{
	if (!enabled) {
		NametableInterface::draw(palette, scanline, pixels, priority);
		return;
	}

	for (int curr_pixel = 0; curr_pixel < 256; ++curr_pixel) {
		coarse_x = curr_pixel / 8;
		fine_x = curr_pixel % 8;
		auto tile_no = data->read_tile_no(coarse_x, coarse_y, table);
		auto attrib_data = data->read_tile_attrib(coarse_x, coarse_y, table);
		uint8_t attrib_shift = calc_attrib_shift(coarse_x, coarse_y);
		auto pattern = get_pattern(tile_no);

		uint8_t temp = attrib_data >> attrib_shift;
		uint8_t attrib = temp & 0x03;
		uint8_t backdrop = (*palette)[0][0];

		uint8_t pal_index = (pattern >> (2 * (8 - fine_x - 1))) & 0x03;//Pixel value from pattern table
		uint8_t pal = (*palette)[attrib][pal_index];
		Color pixel = NES_PAL[pal];

		if (pal_index == 0) {
			(*pixels)[curr_pixel] = NES_PAL[backdrop];
		}
		else {
			(*pixels)[curr_pixel] = pixel;
			(*priority)[curr_pixel] = 1;
		}
	}
	next_y();
}

void HorizontalInterface::set_base_address(uint8_t opt)
{
	switch (opt) {
	case 0: case 1:
		_table = 0;
		break;
	case 2: case 3:
		_table = 1;
		break;
	default:
		assert(false);
	}
}

void HorizontalInterface::begin()
{
	
}

void HorizontalInterface::reset(bool)
{
	coarse_x = fine_x = 0;
	coarse_y = _coarse_y;
	fine_y = _fine_y;
	table = _table;
}


void HorizontalInterface::normalize_address(uint16_t* address, int* table) {

	if (*address < 0x2800) {
		*table = 0;
	}
	else {
		*table = 1;
	}
	*address &= 0x3FF;
}

SingleScreenInterface::SingleScreenInterface(NametableData* data, ReadWriteable* rw)
	:NametableInterface(data, rw)
{
}

uint8_t SingleScreenInterface::read(uint16_t address)
{
	int t;
	normalize_address(&address, &t);
	return data->read((1024 * t) + address);
}

void SingleScreenInterface::write(uint16_t address, uint8_t value)
{
	int t;
	normalize_address(&address, &t);
	data->write((1024 * t) + address, value);
}

void SingleScreenInterface::set_scroll(uint16_t reg, uint8_t fine_x)
{
	_coarse_x = uint8_t(reg & 0x001F);
	_fine_x = fine_x;
	_coarse_y = uint8_t((reg & 0x03E0) >> 5);
	_fine_y = uint8_t((reg & 0x7000) >> 12);
}

void SingleScreenInterface::draw(uint8_t(*palette)[4][4], int scanline, Color(*pixels)[256], uint8_t(*priority)[256])
{
	if (!enabled) {
		NametableInterface::draw(palette, scanline, pixels, priority);
		next_y();
		return;
	}

	for (int curr_pixel = 0; curr_pixel < 256; ++curr_pixel) {

		auto tile_no = data->read_tile_no(coarse_x, coarse_y, table);
		auto attrib_data = data->read_tile_attrib(coarse_x, coarse_y, table);
		uint8_t attrib_shift = calc_attrib_shift(coarse_x, coarse_y);
		auto pattern = get_pattern(tile_no);

		uint8_t temp = attrib_data >> attrib_shift;
		uint8_t attrib = temp & 0x03;
		uint8_t backdrop = (*palette)[0][0];

		uint8_t pal_index = (pattern >> (2 * (8 - fine_x - 1))) & 0x03;//Pixel value from pattern table
		uint8_t pal = (*palette)[attrib][pal_index];
		Color pixel = NES_PAL[pal];

		if (pal_index == 0) {
			(*pixels)[curr_pixel] = NES_PAL[backdrop];
		}
		else {
			(*pixels)[curr_pixel] = pixel;
			(*priority)[curr_pixel] = 1;
		}
		next_x();
	}
	next_y();
}

void SingleScreenInterface::set_base_address(uint8_t opt)
{
	
	  switch (opt) {
	case 0: //case 2:
		//_table = 0;
		break;
	case 1: //case 3:
		//_table = 1;
		break;
	default:
		assert(false);
	}
	
}

void SingleScreenInterface::begin()
{
	coarse_x = _coarse_x;
	fine_x = _fine_x;
	//table = _table;
}

void SingleScreenInterface::reset(bool)
{
	coarse_y = _coarse_y;
	fine_y = _fine_y;
	table = _table;
}

void SingleScreenInterface::set_table(uint8_t t)
{
	_table = t;
}

void SingleScreenInterface::normalize_address(uint16_t* address, int* table) {
	if (*address >= 0x2000 && *address < 0x2400) {
		*table = 0;
	}
	else if (*address >= 0x2400 && *address < 0x2800) {
		*table = 1;
	}
	else {
		*table = this->table;
		//assert(false);
	}
	*address &= 0x3FF;
}

void SingleScreenInterface::next_x()
{
	fine_x += 1;
	if (fine_x == 8) {
		coarse_x += 1;
		fine_x = 0;
	}
	coarse_x %= 32;
}

void SingleScreenInterface::next_y()
{
	fine_y += 1;
	if (fine_y == 8) {
		coarse_y += 1;
		fine_y = 0;
	}
	coarse_y %= 30;
}
