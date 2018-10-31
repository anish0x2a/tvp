/**
 * @file cpu.cpp
 * Defines the CPU class
 */

#include "cpu/cpu.h"
#include "cpu/register/register.h"

#include <functional>

namespace cpu {

// Make some registers!
CPU::CPU(memory::MemoryInterface *memory)
    : a(std::make_unique<Register>()), b(std::make_unique<Register>()),
      c(std::make_unique<Register>()), d(std::make_unique<Register>()),
      e(std::make_unique<Register>()), f(std::make_unique<Register>()),
      h(std::make_unique<Register>()), l(std::make_unique<Register>()),
      af(std::make_unique<PairRegister>(a.get(), f.get())),
      bc(std::make_unique<PairRegister>(b.get(), c.get())),
      de(std::make_unique<PairRegister>(d.get(), e.get())),
      hl(std::make_unique<PairRegister>(h.get(), l.get())),
      sp(std::make_unique<DoubleRegister>()),
      pc(std::make_unique<DoubleRegister>()), memory(memory), halted(false),
      interrupt_enabled(true), branch_taken(false),

      // Initialize the opcode map
      opcode_map({
          // clang-format off
          /* 0x00 */ [=] { op_nop(); },
          /* 0x01 */ [=] { op_ld_dbl(bc.get(), get_inst_dbl()); },
          /* 0x02 */ [=] { op_ld(bc->get(), a->get()); },
          /* 0x03 */ [=] { op_inc_dbl(bc.get()); },
          /* 0x04 */ [=] { op_inc(b.get()); },
          /* 0x05 */ [=] { op_dec(b.get()); },
          /* 0x06 */ [=] { op_ld(b.get(), get_inst_byte()); },
          /* 0x07 */ [=] { op_rlc_a(); },
          /* 0x08 */ [=] { op_ld_dbl(static_cast<Address>(get_inst_dbl()), sp->get()); },
          /* 0x09 */ [=] { op_add_hl(bc->get()); },
          /* 0x0a */ [=] { op_ld(a.get(), memory->read(bc->get())); },
          /* 0x0b */ [=] { op_dec_dbl(bc.get()); },
          /* 0x0c */ [=] { op_inc(c.get()); },
          /* 0x0d */ [=] { op_dec(c.get()); },
          /* 0x0e */ [=] { op_ld(c.get(), get_inst_byte()); },
          /* 0x0f */ [=] { op_rrc_a(); },
          /* 0x10 */ [=] { op_stop(); },
          /* 0x11 */ [=] { op_ld_dbl(de.get(), get_inst_dbl()); },
          /* 0x12 */ [=] { op_ld(de->get(), a->get()); },
          /* 0x13 */ [=] { op_inc_dbl(de.get()); },
          /* 0x14 */ [=] { op_inc(d.get()); },
          /* 0x15 */ [=] { op_dec(d.get()); },
          /* 0x16 */ [=] { op_ld(d.get(), get_inst_byte()); },
          /* 0x17 */ [=] { op_rl_a(); },
          /* 0x18 */ [=] { op_jr(get_inst_byte()); },
          /* 0x19 */ [=] { op_add_hl(de->get()); },
          /* 0x1a */ [=] { op_ld(a.get(), memory->read(de->get())); },
          /* 0x1b */ [=] { op_dec_dbl(de.get()); },
          /* 0x1c */ [=] { op_inc(e.get()); },
          /* 0x1d */ [=] { op_dec(e.get()); },
          /* 0x1e */ [=] { op_ld(e.get(), get_inst_byte()); },
          /* 0x1f */ [=] { op_rr_a(); },
          /* 0x20 */ [=] { op_jr(!f->get_bit(flag::ZERO), get_inst_byte()); },
          /* 0x21 */ [=] { op_ld_dbl(hl.get(), get_inst_dbl()); },
          /* 0x22 */ [=] { op_ldi_addr(hl->get(), a->get()); },
          /* 0x23 */ [=] { op_inc_dbl(hl.get()); },
          /* 0x24 */ [=] { op_inc(h.get()); },
          /* 0x25 */ [=] { op_dec(h.get()); },
          /* 0x26 */ [=] { op_ld(h.get(), get_inst_byte()); },
          /* 0x27 */ [=] { op_daa(); },
          /* 0x28 */ [=] { op_jr(f->get_bit(flag::ZERO), get_inst_byte()); },
          /* 0x29 */ [=] { op_add_hl(hl->get()); },
          /* 0x2a */ [=] { op_ldi_a(memory->read(hl->get())); },
          /* 0x2b */ [=] { op_dec_dbl(hl.get()); },
          /* 0x2c */ [=] { op_inc(l.get()); },
          /* 0x2d */ [=] { op_dec(l.get()); },
          /* 0x2e */ [=] { op_ld(l.get(), get_inst_byte()); },
          /* 0x2f */ [=] { op_cpl(); },
          /* 0x30 */ [=] { op_jr(!f->get_bit(flag::CARRY), get_inst_byte()); },
          /* 0x31 */ [=] { op_ld_dbl(sp.get(), get_inst_dbl()); },
          /* 0x32 */ [=] { op_ldd_addr(static_cast<Address>(hl->get()), a->get()); },
          /* 0x33 */ [=] { op_inc_dbl(sp.get()); },
          /* 0x34 */ [=] { op_inc(static_cast<Address>(hl->get())); },
          /* 0x35 */ [=] { op_dec(static_cast<Address>(hl->get())); },
          /* 0x36 */ [=] { op_ld(static_cast<Address>(hl->get()), get_inst_byte()); },
          /* 0x37 */ [=] { op_scf(); },
          /* 0x38 */ [=] { op_jr(f->get_bit(flag::CARRY), get_inst_byte()); },
          /* 0x39 */ [=] { op_add_hl(sp->get()); },
          /* 0x3a */ [=] { op_ldd_a(memory->read(hl->get())); },
          /* 0x3b */ [=] { op_dec_dbl(sp.get()); },
          /* 0x3c */ [=] { op_inc(a.get()); },
          /* 0x3d */ [=] { op_dec(a.get()); },
          /* 0x3e */ [=] { op_ld(a.get(), get_inst_byte()); },
          /* 0x3f */ [=] { op_ccf(); },
          /* 0x40 */ [=] { op_ld(b.get(), b->get()); },
          /* 0x41 */ [=] { op_ld(b.get(), c->get()); },
          /* 0x42 */ [=] { op_ld(b.get(), d->get()); },
          /* 0x43 */ [=] { op_ld(b.get(), e->get()); },
          /* 0x44 */ [=] { op_ld(b.get(), h->get()); },
          /* 0x45 */ [=] { op_ld(b.get(), l->get()); },
          /* 0x46 */ [=] { op_ld(b.get(), memory->read(hl->get())); },
          /* 0x47 */ [=] { op_ld(b.get(), a->get()); },
          /* 0x48 */ [=] { op_ld(c.get(), b->get()); },
          /* 0x49 */ [=] { op_ld(c.get(), c->get()); },
          /* 0x4a */ [=] { op_ld(c.get(), d->get()); },
          /* 0x4b */ [=] { op_ld(c.get(), e->get()); },
          /* 0x4c */ [=] { op_ld(c.get(), h->get()); },
          /* 0x4d */ [=] { op_ld(c.get(), l->get()); },
          /* 0x4e */ [=] { op_ld(c.get(), memory->read(hl->get())); },
          /* 0x4f */ [=] { op_ld(c.get(), a->get()); },
          /* 0x50 */ [=] { op_ld(d.get(), b->get()); },
          /* 0x51 */ [=] { op_ld(d.get(), c->get()); },
          /* 0x52 */ [=] { op_ld(d.get(), d->get()); },
          /* 0x53 */ [=] { op_ld(d.get(), e->get()); },
          /* 0x54 */ [=] { op_ld(d.get(), h->get()); },
          /* 0x55 */ [=] { op_ld(d.get(), l->get()); },
          /* 0x56 */ [=] { op_ld(d.get(), memory->read(hl->get())); },
          /* 0x57 */ [=] { op_ld(d.get(), a->get()); },
          /* 0x58 */ [=] { op_ld(e.get(), b->get()); },
          /* 0x59 */ [=] { op_ld(e.get(), c->get()); },
          /* 0x5a */ [=] { op_ld(e.get(), d->get()); },
          /* 0x5b */ [=] { op_ld(e.get(), e->get()); },
          /* 0x5c */ [=] { op_ld(e.get(), h->get()); },
          /* 0x5d */ [=] { op_ld(e.get(), l->get()); },
          /* 0x5e */ [=] { op_ld(e.get(), memory->read(hl->get())); },
          /* 0x5f */ [=] { op_ld(e.get(), a->get()); },
          /* 0x60 */ [=] { op_ld(h.get(), b->get()); },
          /* 0x61 */ [=] { op_ld(h.get(), c->get()); },
          /* 0x62 */ [=] { op_ld(h.get(), d->get()); },
          /* 0x63 */ [=] { op_ld(h.get(), e->get()); },
          /* 0x64 */ [=] { op_ld(h.get(), h->get()); },
          /* 0x65 */ [=] { op_ld(h.get(), l->get()); },
          /* 0x66 */ [=] { op_ld(h.get(), memory->read(hl->get())); },
          /* 0x67 */ [=] { op_ld(h.get(), a->get()); },
          /* 0x68 */ [=] { op_ld(l.get(), b->get()); },
          /* 0x69 */ [=] { op_ld(l.get(), c->get()); },
          /* 0x6a */ [=] { op_ld(l.get(), d->get()); },
          /* 0x6b */ [=] { op_ld(l.get(), e->get()); },
          /* 0x6c */ [=] { op_ld(l.get(), h->get()); },
          /* 0x6d */ [=] { op_ld(l.get(), l->get()); },
          /* 0x6e */ [=] { op_ld(l.get(), memory->read(hl->get())); },
          /* 0x6f */ [=] { op_ld(l.get(), a->get()); },
          /* 0x70 */ [=] { op_ld(static_cast<Address>(hl->get()), b->get()); },
          /* 0x71 */ [=] { op_ld(static_cast<Address>(hl->get()), c->get()); },
          /* 0x72 */ [=] { op_ld(static_cast<Address>(hl->get()), d->get()); },
          /* 0x73 */ [=] { op_ld(static_cast<Address>(hl->get()), e->get()); },
          /* 0x74 */ [=] { op_ld(static_cast<Address>(hl->get()), h->get()); },
          /* 0x75 */ [=] { op_ld(static_cast<Address>(hl->get()), l->get()); },
          /* 0x76 */ [=] { op_halt(); },
          /* 0x77 */ [=] { op_ld(static_cast<Address>(hl->get()), a->get()); },
          /* 0x78 */ [=] { op_ld(a.get(), b->get()); },
          /* 0x79 */ [=] { op_ld(a.get(), c->get()); },
          /* 0x7a */ [=] { op_ld(a.get(), d->get()); },
          /* 0x7b */ [=] { op_ld(a.get(), e->get()); },
          /* 0x7c */ [=] { op_ld(a.get(), h->get()); },
          /* 0x7d */ [=] { op_ld(a.get(), l->get()); },
          /* 0x7e */ [=] { op_ld(a.get(), memory->read(hl->get())); },
          /* 0x7f */ [=] { op_ld(a.get(), a->get()); },
          /* 0x80 */ [=] { op_add(b->get()); },
          /* 0x81 */ [=] { op_add(c->get()); },
          /* 0x82 */ [=] { op_add(d->get()); },
          /* 0x83 */ [=] { op_add(e->get()); },
          /* 0x84 */ [=] { op_add(h->get()); },
          /* 0x85 */ [=] { op_add(l->get()); },
          /* 0x86 */ [=] { op_add(memory->read(hl->get())); },
          /* 0x87 */ [=] { op_add(a->get()); },
          /* 0x88 */ [=] { op_adc(b->get()); },
          /* 0x89 */ [=] { op_adc(c->get()); },
          /* 0x8a */ [=] { op_adc(d->get()); },
          /* 0x8b */ [=] { op_adc(e->get()); },
          /* 0x8c */ [=] { op_adc(h->get()); },
          /* 0x8d */ [=] { op_adc(l->get()); },
          /* 0x8e */ [=] { op_adc(memory->read(hl->get())); },
          /* 0x8f */ [=] { op_adc(a->get()); },
          /* 0x90 */ [=] { op_sub(b->get()); },
          /* 0x91 */ [=] { op_sub(c->get()); },
          /* 0x92 */ [=] { op_sub(d->get()); },
          /* 0x93 */ [=] { op_sub(e->get()); },
          /* 0x94 */ [=] { op_sub(h->get()); },
          /* 0x95 */ [=] { op_sub(l->get()); },
          /* 0x96 */ [=] { op_sub(memory->read(hl->get())); },
          /* 0x97 */ [=] { op_sub(a->get()); },
          /* 0x98 */ [=] { op_sbc(b->get()); },
          /* 0x99 */ [=] { op_sbc(c->get()); },
          /* 0x9a */ [=] { op_sbc(d->get()); },
          /* 0x9b */ [=] { op_sbc(e->get()); },
          /* 0x9c */ [=] { op_sbc(h->get()); },
          /* 0x9d */ [=] { op_sbc(l->get()); },
          /* 0x9e */ [=] { op_sbc(memory->read(hl->get())); },
          /* 0x9f */ [=] { op_sbc(a->get()); },
          /* 0xa0 */ [=] { op_and(b->get()); },
          /* 0xa1 */ [=] { op_and(c->get()); },
          /* 0xa2 */ [=] { op_and(d->get()); },
          /* 0xa3 */ [=] { op_and(e->get()); },
          /* 0xa4 */ [=] { op_and(h->get()); },
          /* 0xa5 */ [=] { op_and(l->get()); },
          /* 0xa6 */ [=] { op_and(memory->read(hl->get())); },
          /* 0xa7 */ [=] { op_and(a->get()); },
          /* 0xa8 */ [=] { op_xor(b->get()); },
          /* 0xa9 */ [=] { op_xor(c->get()); },
          /* 0xaa */ [=] { op_xor(d->get()); },
          /* 0xab */ [=] { op_xor(e->get()); },
          /* 0xac */ [=] { op_xor(h->get()); },
          /* 0xad */ [=] { op_xor(l->get()); },
          /* 0xae */ [=] { op_xor(memory->read(hl->get())); },
          /* 0xaf */ [=] { op_xor(a->get()); },
          /* 0xb0 */ [=] { op_or(b->get()); },
          /* 0xb1 */ [=] { op_or(c->get()); },
          /* 0xb2 */ [=] { op_or(d->get()); },
          /* 0xb3 */ [=] { op_or(e->get()); },
          /* 0xb4 */ [=] { op_or(h->get()); },
          /* 0xb5 */ [=] { op_or(l->get()); },
          /* 0xb6 */ [=] { op_or(memory->read(hl->get())); },
          /* 0xb7 */ [=] { op_or(a->get()); },
          /* 0xb8 */ [=] { op_cp(b->get()); },
          /* 0xb9 */ [=] { op_cp(c->get()); },
          /* 0xba */ [=] { op_cp(d->get()); },
          /* 0xbb */ [=] { op_cp(e->get()); },
          /* 0xbc */ [=] { op_cp(h->get()); },
          /* 0xbd */ [=] { op_cp(l->get()); },
          /* 0xbe */ [=] { op_cp(memory->read(hl->get())); },
          /* 0xbf */ [=] { op_cp(a->get()); },
          /* 0xc0 */ [=] { op_ret(!f->get_bit(flag::ZERO)); },
          /* 0xc1 */ [=] { op_pop(bc.get()); },
          /* 0xc2 */ [=] { op_jr(!f->get_bit(flag::ZERO), get_inst_dbl()); },
          /* 0xc3 */ [=] { op_jp(get_inst_dbl()); },
          /* 0xc4 */ [=] { op_call(!f->get_bit(flag::ZERO), get_inst_dbl()); },
          /* 0xc5 */ [=] { op_push(bc.get()); },
          /* 0xc6 */ [=] { op_add(get_inst_byte()); },
          /* 0xc7 */ [=] { op_rst(0x00); },
          /* 0xc8 */ [=] { op_ret(f->get_bit(flag::ZERO)); },
          /* 0xc9 */ [=] { op_ret(); },
          /* 0xca */ [=] { op_jr(f->get_bit(flag::ZERO), get_inst_dbl()); },
          /* 0xcb */ [=] { /* CB Opcodes handled separately */ },
          /* 0xcc */ [=] { op_call(f->get_bit(flag::ZERO), get_inst_dbl()); },
          /* 0xcd */ [=] { op_call(get_inst_dbl()); },
          /* 0xce */ [=] { op_adc(get_inst_byte()); },
          /* 0xcf */ [=] { op_rst(0x08); },
          /* 0xd0 */ [=] { op_ret(!f->get_bit(flag::CARRY)); },
          /* 0xd1 */ [=] { op_pop(de.get()); },
          /* 0xd2 */ [=] { op_jr(!f->get_bit(flag::CARRY), get_inst_dbl()); },
          /* 0xd3 */ [=] { /* UNDEFINED */ },
          /* 0xd4 */ [=] { op_call(!f->get_bit(flag::CARRY), get_inst_dbl()); },
          /* 0xd5 */ [=] { op_push(de.get()); },
          /* 0xd6 */ [=] { op_sub(get_inst_byte()); },
          /* 0xd7 */ [=] { op_rst(0x10); },
          /* 0xd8 */ [=] { op_ret(f->get_bit(flag::CARRY)); },
          /* 0xd9 */ [=] { op_reti(); },
          /* 0xda */ [=] { op_jr(f->get_bit(flag::CARRY), get_inst_dbl()); },
          /* 0xdb */ [=] { /* UNDEFINED */ },
          /* 0xdc */ [=] { op_call(f->get_bit(flag::CARRY), get_inst_dbl()); },
          /* 0xdd */ [=] { /* UNDEFINED */ },
          /* 0xde */ [=] { op_sbc(get_inst_byte()); },
          /* 0xdf */ [=] { op_rst(0x18); },
          /* 0xe0 */ [=] { op_ldh_addr(0xFF00 + get_inst_byte(), a->get()); },
          /* 0xe1 */ [=] { op_pop(hl.get()); },
          /* 0xe2 */ [=] { op_ld(static_cast<Address>(0x00FF + c->get()), a->get()); },
          /* 0xe3 */ [=] { /* UNDEFINED */ },
          /* 0xe4 */ [=] { /* UNDEFINED */ },
          /* 0xe5 */ [=] { op_push(hl.get()); },
          /* 0xe6 */ [=] { op_and(get_inst_byte()); },
          /* 0xe7 */ [=] { op_rst(0x20); },
          /* 0xe8 */ [=] { op_add_sp(static_cast<int8_t>(get_inst_byte())); },
          /* 0xe9 */ [=] { op_jp(hl->get()); },
          /* 0xea */ [=] { op_ld_dbl(static_cast<Address>(get_inst_dbl()), a->get()); },
          /* 0xeb */ [=] { /* UNDEFINED */ },
          /* 0xec */ [=] { /* UNDEFINED */ },
          /* 0xed */ [=] { /* UNDEFINED */ },
          /* 0xee */ [=] { op_xor(get_inst_byte()); },
          /* 0xef */ [=] { op_rst(0x28); },
          /* 0xf0 */ [=] { op_ldh_a(0xFF00 + get_inst_byte()); },
          /* 0xf1 */ [=] { op_pop(af.get()); },
          /* 0xf2 */ [=] { op_ld(a.get(), memory->read(0xFF00 + c->get())); },
          /* 0xf3 */ [=] { op_di(); },
          /* 0xf4 */ [=] { /* UNDEFINED */ },
          /* 0xf5 */ [=] { op_push(af.get()); },
          /* 0xf6 */ [=] { op_or(get_inst_byte()); },
          /* 0xf7 */ [=] { op_rst(0x30); },
          /* 0xf8 */ [=] { op_ld_hl_sp_offset(static_cast<int8_t>(get_inst_byte())); },
          /* 0xf9 */ [=] { op_ld_dbl(sp.get(), hl->get()); },
          /* 0xfa */ [=] { op_ld(a.get(), memory->read(get_inst_dbl())); },
          /* 0xfb */ [=] { op_di(); },
          /* 0xfc */ [=] { /* UNDEFINED */ },
          /* 0xfd */ [=] { /* UNDEFINED */ },
          /* 0xfe */ [=] { op_cp(get_inst_byte()); },
          /* 0xff */ [=] { op_rst(0x38); },
          // clang-format on
      }),

      // Initialize the CB opcode map
      cb_opcode_map({
          // clang-format off
          /* 0x00 */ [=] { op_rlc(b.get()); },
          /* 0x01 */ [=] { op_rlc(c.get()); },
          /* 0x02 */ [=] { op_rlc(d.get()); },
          /* 0x03 */ [=] { op_rlc(e.get()); },
          /* 0x04 */ [=] { op_rlc(h.get()); },
          /* 0x05 */ [=] { op_rlc(l.get()); },
          /* 0x06 */ [=] { op_rlc(static_cast<Address>(hl->get())); },
          /* 0x07 */ [=] { op_rlc(a.get()); },
          /* 0x08 */ [=] { op_rrc(b.get()); },
          /* 0x09 */ [=] { op_rrc(c.get()); },
          /* 0x0a */ [=] { op_rrc(d.get()); },
          /* 0x0b */ [=] { op_rrc(e.get()); },
          /* 0x0c */ [=] { op_rrc(h.get()); },
          /* 0x0d */ [=] { op_rrc(l.get()); },
          /* 0x0e */ [=] { op_rrc(static_cast<Address>(hl->get())); },
          /* 0x0f */ [=] { op_rrc(a.get()); },
          /* 0x10 */ [=] { op_rl(b.get()); },
          /* 0x11 */ [=] { op_rl(c.get()); },
          /* 0x12 */ [=] { op_rl(d.get()); },
          /* 0x13 */ [=] { op_rl(e.get()); },
          /* 0x14 */ [=] { op_rl(h.get()); },
          /* 0x15 */ [=] { op_rl(l.get()); },
          /* 0x16 */ [=] { op_rl(static_cast<Address>(hl->get())); },
          /* 0x17 */ [=] { op_rl(a.get()); },
          /* 0x18 */ [=] { op_rr(b.get()); },
          /* 0x19 */ [=] { op_rr(c.get()); },
          /* 0x1a */ [=] { op_rr(d.get()); },
          /* 0x1b */ [=] { op_rr(e.get()); },
          /* 0x1c */ [=] { op_rr(h.get()); },
          /* 0x1d */ [=] { op_rr(l.get()); },
          /* 0x1e */ [=] { op_rr(static_cast<Address>(hl->get())); },
          /* 0x1f */ [=] { op_rr(a.get()); },
          /* 0x20 */ [=] { op_sla(b.get()); },
          /* 0x21 */ [=] { op_sla(c.get()); },
          /* 0x22 */ [=] { op_sla(d.get()); },
          /* 0x23 */ [=] { op_sla(e.get()); },
          /* 0x24 */ [=] { op_sla(h.get()); },
          /* 0x25 */ [=] { op_sla(l.get()); },
          /* 0x26 */ [=] { op_sla(static_cast<Address>(hl->get())); },
          /* 0x27 */ [=] { op_sla(a.get()); },
          /* 0x28 */ [=] { op_sra(b.get()); },
          /* 0x29 */ [=] { op_sra(c.get()); },
          /* 0x2a */ [=] { op_sra(d.get()); },
          /* 0x2b */ [=] { op_sra(e.get()); },
          /* 0x2c */ [=] { op_sra(h.get()); },
          /* 0x2d */ [=] { op_sra(l.get()); },
          /* 0x2e */ [=] { op_sra(static_cast<Address>(hl->get())); },
          /* 0x2f */ [=] { op_sra(a.get()); },
          /* 0x30 */ [=] { op_swap(b.get()); },
          /* 0x31 */ [=] { op_swap(c.get()); },
          /* 0x32 */ [=] { op_swap(d.get()); },
          /* 0x33 */ [=] { op_swap(e.get()); },
          /* 0x34 */ [=] { op_swap(h.get()); },
          /* 0x35 */ [=] { op_swap(l.get()); },
          /* 0x36 */ [=] { op_swap(static_cast<Address>(hl->get())); },
          /* 0x37 */ [=] { op_swap(a.get()); },
          /* 0x38 */ [=] { op_srl(b.get()); },
          /* 0x39 */ [=] { op_srl(c.get()); },
          /* 0x3a */ [=] { op_srl(d.get()); },
          /* 0x3b */ [=] { op_srl(e.get()); },
          /* 0x3c */ [=] { op_srl(h.get()); },
          /* 0x3d */ [=] { op_srl(l.get()); },
          /* 0x3e */ [=] { op_srl(static_cast<Address>(hl->get())); },
          /* 0x3f */ [=] { op_srl(a.get()); },
          /* 0x40 */ [=] { op_bit(b.get(), 0); },
          /* 0x41 */ [=] { op_bit(c.get(), 0); },
          /* 0x42 */ [=] { op_bit(d.get(), 0); },
          /* 0x43 */ [=] { op_bit(e.get(), 0); },
          /* 0x44 */ [=] { op_bit(h.get(), 0); },
          /* 0x45 */ [=] { op_bit(l.get(), 0); },
          /* 0x46 */ [=] { op_bit(memory->read(hl->get()), 0); },
          /* 0x47 */ [=] { op_bit(a.get(), 0); },
          /* 0x48 */ [=] { op_bit(b.get(), 1); },
          /* 0x49 */ [=] { op_bit(c.get(), 1); },
          /* 0x4a */ [=] { op_bit(d.get(), 1); },
          /* 0x4b */ [=] { op_bit(e.get(), 1); },
          /* 0x4c */ [=] { op_bit(h.get(), 1); },
          /* 0x4d */ [=] { op_bit(l.get(), 1); },
          /* 0x4e */ [=] { op_bit(memory->read(hl->get()), 1); },
          /* 0x4f */ [=] { op_bit(a.get(), 1); },
          /* 0x50 */ [=] { op_bit(b.get(), 2); },
          /* 0x51 */ [=] { op_bit(c.get(), 2); },
          /* 0x52 */ [=] { op_bit(d.get(), 2); },
          /* 0x53 */ [=] { op_bit(e.get(), 2); },
          /* 0x54 */ [=] { op_bit(h.get(), 2); },
          /* 0x55 */ [=] { op_bit(l.get(), 2); },
          /* 0x56 */ [=] { op_bit(memory->read(hl->get()), 2); },
          /* 0x57 */ [=] { op_bit(a.get(), 2); },
          /* 0x58 */ [=] { op_bit(b.get(), 3); },
          /* 0x59 */ [=] { op_bit(c.get(), 3); },
          /* 0x5a */ [=] { op_bit(d.get(), 3); },
          /* 0x5b */ [=] { op_bit(e.get(), 3); },
          /* 0x5c */ [=] { op_bit(h.get(), 3); },
          /* 0x5d */ [=] { op_bit(l.get(), 3); },
          /* 0x5e */ [=] { op_bit(memory->read(hl->get()), 3); },
          /* 0x5f */ [=] { op_bit(a.get(), 3); },
          /* 0x60 */ [=] { op_bit(b.get(), 4); },
          /* 0x61 */ [=] { op_bit(c.get(), 4); },
          /* 0x62 */ [=] { op_bit(d.get(), 4); },
          /* 0x63 */ [=] { op_bit(e.get(), 4); },
          /* 0x64 */ [=] { op_bit(h.get(), 4); },
          /* 0x65 */ [=] { op_bit(l.get(), 4); },
          /* 0x66 */ [=] { op_bit(memory->read(hl->get()), 4); },
          /* 0x67 */ [=] { op_bit(a.get(), 4); },
          /* 0x68 */ [=] { op_bit(b.get(), 5); },
          /* 0x69 */ [=] { op_bit(c.get(), 5); },
          /* 0x6a */ [=] { op_bit(d.get(), 5); },
          /* 0x6b */ [=] { op_bit(e.get(), 5); },
          /* 0x6c */ [=] { op_bit(h.get(), 5); },
          /* 0x6d */ [=] { op_bit(l.get(), 5); },
          /* 0x6e */ [=] { op_bit(memory->read(hl->get()), 5); },
          /* 0x6f */ [=] { op_bit(a.get(), 5); },
          /* 0x70 */ [=] { op_bit(b.get(), 6); },
          /* 0x71 */ [=] { op_bit(c.get(), 6); },
          /* 0x72 */ [=] { op_bit(d.get(), 6); },
          /* 0x73 */ [=] { op_bit(e.get(), 6); },
          /* 0x74 */ [=] { op_bit(h.get(), 6); },
          /* 0x75 */ [=] { op_bit(l.get(), 6); },
          /* 0x76 */ [=] { op_bit(memory->read(hl->get()), 6); },
          /* 0x77 */ [=] { op_bit(a.get(), 6); },
          /* 0x78 */ [=] { op_bit(b.get(), 7); },
          /* 0x79 */ [=] { op_bit(c.get(), 7); },
          /* 0x7a */ [=] { op_bit(d.get(), 7); },
          /* 0x7b */ [=] { op_bit(e.get(), 7); },
          /* 0x7c */ [=] { op_bit(h.get(), 7); },
          /* 0x7d */ [=] { op_bit(l.get(), 7); },
          /* 0x7e */ [=] { op_bit(memory->read(hl->get()), 7); },
          /* 0x7f */ [=] { op_bit(a.get(), 7); },
          /* 0x80 */ [=] { op_res(b.get(), 0); },
          /* 0x81 */ [=] { op_res(c.get(), 0); },
          /* 0x82 */ [=] { op_res(d.get(), 0); },
          /* 0x83 */ [=] { op_res(e.get(), 0); },
          /* 0x84 */ [=] { op_res(h.get(), 0); },
          /* 0x85 */ [=] { op_res(l.get(), 0); },
          /* 0x86 */ [=] { op_res(hl->get(), 0); },
          /* 0x87 */ [=] { op_res(a.get(), 0); },
          /* 0x88 */ [=] { op_res(b.get(), 1); },
          /* 0x89 */ [=] { op_res(c.get(), 1); },
          /* 0x8a */ [=] { op_res(d.get(), 1); },
          /* 0x8b */ [=] { op_res(e.get(), 1); },
          /* 0x8c */ [=] { op_res(h.get(), 1); },
          /* 0x8d */ [=] { op_res(l.get(), 1); },
          /* 0x8e */ [=] { op_res(hl->get(), 1); },
          /* 0x8f */ [=] { op_res(a.get(), 1); },
          /* 0x90 */ [=] { op_res(b.get(), 2); },
          /* 0x91 */ [=] { op_res(c.get(), 2); },
          /* 0x92 */ [=] { op_res(d.get(), 2); },
          /* 0x93 */ [=] { op_res(e.get(), 2); },
          /* 0x94 */ [=] { op_res(h.get(), 2); },
          /* 0x95 */ [=] { op_res(l.get(), 2); },
          /* 0x96 */ [=] { op_res(hl->get(), 2); },
          /* 0x97 */ [=] { op_res(a.get(), 2); },
          /* 0x98 */ [=] { op_res(b.get(), 3); },
          /* 0x99 */ [=] { op_res(c.get(), 3); },
          /* 0x9a */ [=] { op_res(d.get(), 3); },
          /* 0x9b */ [=] { op_res(e.get(), 3); },
          /* 0x9c */ [=] { op_res(h.get(), 3); },
          /* 0x9d */ [=] { op_res(l.get(), 3); },
          /* 0x9e */ [=] { op_res(hl->get(), 3); },
          /* 0x9f */ [=] { op_res(a.get(), 3); },
          /* 0xa0 */ [=] { op_res(b.get(), 4); },
          /* 0xa1 */ [=] { op_res(c.get(), 4); },
          /* 0xa2 */ [=] { op_res(d.get(), 4); },
          /* 0xa3 */ [=] { op_res(e.get(), 4); },
          /* 0xa4 */ [=] { op_res(h.get(), 4); },
          /* 0xa5 */ [=] { op_res(l.get(), 4); },
          /* 0xa6 */ [=] { op_res(hl->get(), 4); },
          /* 0xa7 */ [=] { op_res(a.get(), 4); },
          /* 0xa8 */ [=] { op_res(b.get(), 5); },
          /* 0xa9 */ [=] { op_res(c.get(), 5); },
          /* 0xaa */ [=] { op_res(d.get(), 5); },
          /* 0xab */ [=] { op_res(e.get(), 5); },
          /* 0xac */ [=] { op_res(h.get(), 5); },
          /* 0xad */ [=] { op_res(l.get(), 5); },
          /* 0xae */ [=] { op_res(hl->get(), 5); },
          /* 0xaf */ [=] { op_res(a.get(), 5); },
          /* 0xb0 */ [=] { op_res(b.get(), 6); },
          /* 0xb1 */ [=] { op_res(c.get(), 6); },
          /* 0xb2 */ [=] { op_res(d.get(), 6); },
          /* 0xb3 */ [=] { op_res(e.get(), 6); },
          /* 0xb4 */ [=] { op_res(h.get(), 6); },
          /* 0xb5 */ [=] { op_res(l.get(), 6); },
          /* 0xb6 */ [=] { op_res(hl->get(), 6); },
          /* 0xb7 */ [=] { op_res(a.get(), 6); },
          /* 0xb8 */ [=] { op_res(b.get(), 7); },
          /* 0xb9 */ [=] { op_res(c.get(), 7); },
          /* 0xba */ [=] { op_res(d.get(), 7); },
          /* 0xbb */ [=] { op_res(e.get(), 7); },
          /* 0xbc */ [=] { op_res(h.get(), 7); },
          /* 0xbd */ [=] { op_res(l.get(), 7); },
          /* 0xbe */ [=] { op_res(hl->get(), 7); },
          /* 0xbf */ [=] { op_res(a.get(), 7); },
          /* 0xc0 */ [=] { op_set(b.get(), 0); },
          /* 0xc1 */ [=] { op_set(c.get(), 0); },
          /* 0xc2 */ [=] { op_set(d.get(), 0); },
          /* 0xc3 */ [=] { op_set(e.get(), 0); },
          /* 0xc4 */ [=] { op_set(h.get(), 0); },
          /* 0xc5 */ [=] { op_set(l.get(), 0); },
          /* 0xc6 */ [=] { op_set(hl->get(), 0); },
          /* 0xc7 */ [=] { op_set(a.get(), 0); },
          /* 0xc8 */ [=] { op_set(b.get(), 1); },
          /* 0xc9 */ [=] { op_set(c.get(), 1); },
          /* 0xca */ [=] { op_set(d.get(), 1); },
          /* 0xcb */ [=] { op_set(e.get(), 1); },
          /* 0xcc */ [=] { op_set(h.get(), 1); },
          /* 0xcd */ [=] { op_set(l.get(), 1); },
          /* 0xce */ [=] { op_set(hl->get(), 1); },
          /* 0xcf */ [=] { op_set(a.get(), 1); },
          /* 0xd0 */ [=] { op_set(b.get(), 2); },
          /* 0xd1 */ [=] { op_set(c.get(), 2); },
          /* 0xd2 */ [=] { op_set(d.get(), 2); },
          /* 0xd3 */ [=] { op_set(e.get(), 2); },
          /* 0xd4 */ [=] { op_set(h.get(), 2); },
          /* 0xd5 */ [=] { op_set(l.get(), 2); },
          /* 0xd6 */ [=] { op_set(hl->get(), 2); },
          /* 0xd7 */ [=] { op_set(a.get(), 2); },
          /* 0xd8 */ [=] { op_set(b.get(), 3); },
          /* 0xd9 */ [=] { op_set(c.get(), 3); },
          /* 0xda */ [=] { op_set(d.get(), 3); },
          /* 0xdb */ [=] { op_set(e.get(), 3); },
          /* 0xdc */ [=] { op_set(h.get(), 3); },
          /* 0xdd */ [=] { op_set(l.get(), 3); },
          /* 0xde */ [=] { op_set(hl->get(), 3); },
          /* 0xdf */ [=] { op_set(a.get(), 3); },
          /* 0xe0 */ [=] { op_set(b.get(), 4); },
          /* 0xe1 */ [=] { op_set(c.get(), 4); },
          /* 0xe2 */ [=] { op_set(d.get(), 4); },
          /* 0xe3 */ [=] { op_set(e.get(), 4); },
          /* 0xe4 */ [=] { op_set(h.get(), 4); },
          /* 0xe5 */ [=] { op_set(l.get(), 4); },
          /* 0xe6 */ [=] { op_set(hl->get(), 4); },
          /* 0xe7 */ [=] { op_set(a.get(), 4); },
          /* 0xe8 */ [=] { op_set(b.get(), 5); },
          /* 0xe9 */ [=] { op_set(c.get(), 5); },
          /* 0xea */ [=] { op_set(d.get(), 5); },
          /* 0xeb */ [=] { op_set(e.get(), 5); },
          /* 0xec */ [=] { op_set(h.get(), 5); },
          /* 0xed */ [=] { op_set(l.get(), 5); },
          /* 0xee */ [=] { op_set(hl->get(), 5); },
          /* 0xef */ [=] { op_set(a.get(), 5); },
          /* 0xf0 */ [=] { op_set(b.get(), 6); },
          /* 0xf1 */ [=] { op_set(c.get(), 6); },
          /* 0xf2 */ [=] { op_set(d.get(), 6); },
          /* 0xf3 */ [=] { op_set(e.get(), 6); },
          /* 0xf4 */ [=] { op_set(h.get(), 6); },
          /* 0xf5 */ [=] { op_set(l.get(), 6); },
          /* 0xf6 */ [=] { op_set(hl->get(), 6); },
          /* 0xf7 */ [=] { op_set(a.get(), 6); },
          /* 0xf8 */ [=] { op_set(b.get(), 7); },
          /* 0xf9 */ [=] { op_set(c.get(), 7); },
          /* 0xfa */ [=] { op_set(d.get(), 7); },
          /* 0xfb */ [=] { op_set(e.get(), 7); },
          /* 0xfc */ [=] { op_set(h.get(), 7); },
          /* 0xfd */ [=] { op_set(l.get(), 7); },
          /* 0xfe */ [=] { op_set(hl->get(), 7); },
          /* 0xff */ [=] { op_set(a.get(), 7); },
          // clang-format on
      }),

      // Cycles per instruction
      // clang-format off
      cycles({
            16, 12,  8,  8, 16, 16,  8, 16, 20,  8,  8,  8, 16, 16,  8, 16,
            16, 12,  8,  8, 16, 16,  8, 16, 12,  8,  8,  8, 16, 16,  8, 16,
             8, 12,  8,  8, 16, 16,  8, 16,  8,  8,  8,  8, 16, 16,  8, 16,
             8, 12,  8,  8, 12, 12, 12, 16,  8,  8,  8,  8, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
             8,  8,  8,  8,  8,  8, 16,  8, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
             8, 12, 12, 16, 12, 16,  8, 16,  8, 16, 12,  0, 12, 24,  8, 16,
             8, 12, 12,  0, 12, 16,  8, 16,  8, 16, 12,  0, 12,  0,  8, 16,
            12, 12,  8,  0,  0, 16,  8, 16, 16, 16, 16,  0,  0,  0,  8, 16,
            12, 12,  8, 16,  0, 16,  8, 16, 12,  8, 16, 16,  0,  0,  8, 16
      }),
      cycles_branched({
            16, 12,  8,  8, 16, 16,  8, 16, 20,  8,  8,  8, 16, 16,  8, 16,
            16, 12,  8,  8, 16, 16,  8, 16, 12,  8,  8,  8, 16, 16,  8, 16,
            12, 12,  8,  8, 16, 16,  8, 16, 12,  8,  8,  8, 16, 16,  8, 16,
            12, 12,  8,  8, 12, 12, 12, 16, 12,  8,  8,  8, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
             8,  8,  8,  8,  8,  8, 16,  8, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            16, 16, 16, 16, 16, 16,  8, 16, 16, 16, 16, 16, 16, 16,  8, 16,
            20, 12, 16, 16,  6, 16,  8, 16, 20, 16, 16,  0,  6,  6,  8, 16,
            20, 12, 16,  0,  6, 16,  8, 16, 20, 16, 16,  0,  6,  0,  8, 16,
            12, 12,  8,  0,  0, 16,  8, 16, 16, 16, 16,  0,  0,  0,  8, 16,
            12, 12,  8, 16,  0, 16,  8, 16, 12,  8, 16, 16,  0,  0,  8, 16
      }),
      cycles_cb({
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,
             8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,
             8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,
             8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
             8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8
        })
// clang-format on
{}

}

ClockCycles CPU::execute(uint8_t opcode, uint16_t pc) { return 0; }

uint8_t CPU::get_inst_byte() const {
	auto byte = memory->read(pc->get());
	(*pc)++;
	return byte;
};

uint16_t CPU::get_inst_dbl() const {
	auto lower = memory->read(pc->get());
	(*pc)++;
	auto upper = memory->read(pc->get());
	(*pc)++;

	auto result = static_cast<uint16_t>((upper << 8) | lower);
	return result;
};

} // namespace cpu
