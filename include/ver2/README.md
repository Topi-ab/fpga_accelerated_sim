# templated class sturcure to assist getting / settings bit-fields for with HW registers

## hw_access

Bottomline is </i>hw_access</i> class (`hw_access_aarch64.h` as an example) which opens the HW channel with constructor, and provides:

=> <b>This class needs to reside in memory as an object.</b><br>
=> <b>User may need to adapt this file to his needs.</b>

- `wr_word_t` as a type which is to be written at single call (relfects the underlying HW capabilities).
- `rd_word_t` as a type which is read from hw in single call.
- `wr_raw()` which writes a single word to word address (not byte address).
- `rd_raw()` which reads a single word from word address.
- `wr()` which writes a single word to offsett address (actual offset is private in this class).
- `rd()` which reads a single word from offset address (wr and rd offsets can be different).

The example `hw_access_aarch64.h` supports word types from uint8_t upto __uint128_t, separate for read and write.

This is a class that user needs to create/modify if method to access to HW register is different.

## shadow

=> <b>This class needs to reside in memory as an object.</b><br>
=> <b>User does not need to modify this file.</b>

The next layer up in call graph is <i>shadow</i> class (`shadow.h`), which creates cache array for both read ad write actions.

Each write cache entry is type of `hw_access::wr_word_t` and equivalent for read cache entries.

Both write and read caches have per-entry dirty flags.

For write, the flag is marked dirty as soon as the cache entry is written to. 
When all relevant data has been written to cache, call to `wr_flush()` writes 
all dirty entries to <i>hw_access</i> interface, and all dirty flags are marked as non-dirty.

For read, all dirty flags are marked as dirty in rd_flush() call, and when word is read, 
the code checks corresponding word's dirty flag. Dirty means the data is read from <i>hw_access</i> interface 
and stored to cache entry, and dirty is marked non-dirty. If the dirty flag is non-dirty, then read code returns data from cache entry and does not call <i>hw_access</i> to read the data.

This class provides:

- `wr_word_t` as a type of single write call. This is grabbed from <i>hw_access</i>.
- `rd_word_t` as a type of single read call.hw_access_aarch64.h`.
- `write()` which writes data to the wr-cache entry and marks entry dirty. Data mask is used to tell which bits are to be modified.
- `wr_flush()` which dumps the wr-cache to hw registers and clears dirty flags.
- `read()` which reads data from rd-cache or from hw-interface, and clears entry's dirty flag.
- `rd_flush()` which sets rd-cache dirty flags.
- `wr_raw()` which writes directly to hw register without cache.
- `rd_raw()` which reads directly from hw register without cache.

## bit_slicer

=> <b>This class needs to reside in memory as an object.</b><br>
=> <b>User does not need to modify this file.</b>

The next layer up in call graph is <i>bit_slicer</i>. This provides write and read calls to write random bit widths to bit-addressed address space.

The code splits single bit-vector write/read to one or more writes/reads to <i>shadow</i> class.

This class provides:

- `write_bits()` for any user-defined type supporting required bit operations. This will split the write to several <i>hw_access::wr_word_t</i> chunks and calls <i>shadow.write()</i>.
- `wr_hw()` which writes a single hw-word to <i>hw_accces</i>.
- `read_bits()` for any user-defined type supporting required bit operations. This will split the reea to several <i>hw_access::rd_word_t</i> reads from <i>shadow.read()</i> and merges the data to complete bit-vector.
- `rd_hw()` which reads a single hw-word from <i>hw_accces</i>.
- `wr_flush()` which calls underlying <i>shadow.wr_flush()</i>.
- `rd_flush()` which calls underlying <i>shadow.rd_flush()</i>.

## fields

=> <b>This class is a template class only, and does not provide any object to wrok on.</b><br>
=> <b>User does not need to modify this file.</b>

This class converts user-defined fields specifications to bit-addresses and bit-widths.

Data sematics is 
- spec (specification) telling field names and their widhts.
- desc (descriptor) telling field's bit-address and bit-width. 

This class provides:

- `wr_fields` as a type (union) where all user defined writable (input to FPGA DUT) fields are listed.
- `rd_fields` as a type (union) where all user defined readable (output from FPGA DUT) fields are listed.
- `spec_bits()` as a consteval (calculated on compile time) function telling how many bits single spec has.
- `wr_desc()` to get a desc from wr field name.
- `rd_desc()` to get a desc from rd field name.

This class also double-checks that user supplied fields and their widths-definitions have equal number of entries.

## user defined fields

=> <b>This class is a template class only, and does not provide any object to wrok on.</b><br>
=> <b>User may need to adapt this file to his needs.</b>

`fields_linkruncca.h` is an example file, which will be user-modified to meet the FPGA DUT requirements (I/O definitions).

The example code uses `FpgaGenerics_linkrucca` as a means to calculate interface bit-widths based on configuration. The corresponding VHDL code has the exact same calculation for DUT bit witdhts.

User needs to provide:
- `enum class wr_fields: size_t...` definiton for all wr fields. The last entry needs to be `END_OF_FIELDS`.
- `enum class rd_fields: size_t...` definiton for all wr fields. The last entry needs to be `END_OF_FIELDS`.
- `get_wr_specs()` contents listing all bit widths for each field. The list needs to be in order they appear in FPGA DUT interface.
- `get_rd_specs()` contents listing all bit widths for each field. The list needs to be in order they appear in FPGA DUT interface.

The FPGA code has user-modifiable serializer & deserializer procedures to match the bit packing / unpacking in the emulator wrapper.

## emulator_fields

=> <b>This class needs to reside in memory as an object.</b><br>
=> <b>User does not need to modify this file.</b>

This class combines all other classes together, and provides a single 
interface to user application to write and read bit-fields and direct hw registers.

This class provides:
- `wr_field()` to write single field using user-defined address name (union entry).
- `rd_field()` to read single field using user-defined address name (union entry).
- `wr_flush()` to write all dirty data to actual HW.
- `rd_flush()` to dirty all read caches so that next reads are guaranteed to read from actual HW.
- `wr_raw()` to write directly to hw.
- `rd_raw()` to read directly from hw.

Example code to use:
```
constexpr FpgaGenerics_linkruncca llcca_gens{
    .X_SIZE=1024,
    .Y_BITS=16,
};

using BackendType = hw_access_aarch64;
using app_fields_t = fields_linkruncca<llcca_gens>;
using emulator_t = emulator_fields<BackendType, app_fields_t>;
using wr_add = typename emulator_t::wr_fields;
using rd_add = typename emulator_t::rd_fields;

BackendType hw("/dev/uio4");
emulator_t emulator(hw);

int data_valid;

emulator.wr_field(wr_add::RST, 1);
emulator.wr_field(wr_add::DATAVALID, 1);
emulator.wr_flush();
emulator.wr_raw(0, 1); // Creates a single clock pulse for DUT.
emulator.wr_field(wr_add::RST, 0);
emulator.wr_field(wr_add::DATAVALID, 0);
emulator.wr_flush();
emulator.wr_raw(0, 1); // Creates a single clock pulse for DUT.
emulator.rd_flush();
emulator.rd_field(rd_add::VALID, data_valid);
if(data_valid)
    ...
```
