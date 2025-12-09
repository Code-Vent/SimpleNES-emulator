#ifndef BACKGROUND_H
#define BACKGROUND_H
#include<stdint.h>
#include<raylib/raylib.h>

enum class BackgroundTable {
	TABLE_0000 = 0x0000,
	TABLE_1000 = 0x1000,
};

struct ReadWriteable {
	virtual uint8_t read(uint16_t address) const = 0;
	virtual void write(uint16_t address, uint8_t data) = 0;
};

class NametableData {
public:
	void    write(uint16_t address, uint8_t data);
	uint8_t read(uint16_t address) const;
	//Utility funcs for background rendering
	uint8_t read_tile_no(int x, int y, uint8_t table/*0 or 1*/) const;
	uint8_t read_tile_attrib(int x, int y, uint8_t table/*0 or 1*/) const;
private:
	uint8_t table_data[0x800];
};

struct NametableInterface {
	NametableInterface(NametableData* data, ReadWriteable* rw = nullptr);
	virtual uint8_t read(uint16_t address) = 0;
	virtual void    write(uint16_t address, uint8_t value) = 0;
	virtual void    set_scroll(uint16_t reg, uint8_t fine_x) = 0;
	virtual void    draw(uint8_t(*palette)[4][4],
		                 int scanline,
		                 Color(*pixels)[256],
		                 uint8_t(*priority)[256]);
	virtual void    set_base_address(uint8_t opt) = 0;
	virtual void    begin() = 0;
	virtual void    reset(bool) = 0;
	bool            enabled;
	bool            show_leftmost;
	BackgroundTable tbl;
protected:
	ReadWriteable* rw;
	uint8_t        calc_attrib_shift(int x, int y);
	uint16_t       get_pattern(int tile_no) const;
	virtual void   next_x();
	virtual void   next_y();
	NametableData* data;
	uint8_t        coarse_x;
	uint8_t        fine_x;
	uint8_t        coarse_y;
	uint8_t        fine_y;
	uint8_t        _coarse_x;
	uint8_t        _fine_x;
	uint8_t        _coarse_y;
	uint8_t        _fine_y;
	uint8_t        table;
	uint8_t        _table;
};

struct VerticalInterface : public NametableInterface {
	VerticalInterface(NametableData* data, ReadWriteable* rw);
	uint8_t read(uint16_t address) override;
	void    write(uint16_t address, uint8_t data) override;
	void    set_scroll(uint16_t reg, uint8_t fine_x) override;
	void    draw(uint8_t(*palette)[4][4],
		         int scanline,
		         Color(*pixels)[256],
		         uint8_t(*priority)[256]) override;
	void    set_base_address(uint8_t opt) override;
	void    begin() override;
	void    reset(bool) override;
private:
	void    normalize_address(uint16_t* address, int* table);
};

struct HorizontalInterface : public NametableInterface {
	HorizontalInterface(NametableData* data, ReadWriteable* rw);
	uint8_t read(uint16_t address) override;
	void    write(uint16_t address, uint8_t data) override;
	void    set_scroll(uint16_t reg, uint8_t fine_x) override;
	void    draw(uint8_t(*palette)[4][4],
		         int scanline,
		         Color(*pixels)[256],
		         uint8_t(*priority)[256]) override;
	void    set_base_address(uint8_t opt) override;
	void    begin() override;
	void    reset(bool) override;
private:
	void    normalize_address(uint16_t* address, int* table);
};

struct SingleScreenInterface : public NametableInterface {
	SingleScreenInterface(NametableData* data, ReadWriteable* rw);
	uint8_t read(uint16_t address) override;
	void    write(uint16_t address, uint8_t data) override;
	void    set_scroll(uint16_t reg, uint8_t fine_x) override;
	void    draw(uint8_t(*palette)[4][4],
		         int scanline,
		         Color(*pixels)[256],
		         uint8_t(*priority)[256]) override;
	void    set_base_address(uint8_t opt) override;
	void    begin() override;
	void    reset(bool) override;
	void    set_table(uint8_t table /**0 0r 1*/);
private:
	void    normalize_address(uint16_t* address, int* table);
	void    next_x() override;
	void    next_y() override;
};

#endif // !BACKGROUND_H
