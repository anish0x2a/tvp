/**
 * @file cpu.cpp
 * Defines the CPU class
 */

#include "cpu/cpu.h"
#include "cpu/register/register.h"
#include "util/helpers.h"
#include "util/log.h"

#include <functional>

namespace cpu {

// Make some registers!
CPU::CPU(
    std::unique_ptr<RegisterInterface> a, std::unique_ptr<RegisterInterface> b,
    std::unique_ptr<RegisterInterface> c, std::unique_ptr<RegisterInterface> d,
    std::unique_ptr<RegisterInterface> e, std::unique_ptr<RegisterInterface> f,
    std::unique_ptr<RegisterInterface> h, std::unique_ptr<RegisterInterface> l,
    std::unique_ptr<DoubleRegisterInterface> af,
    std::unique_ptr<DoubleRegisterInterface> bc,
    std::unique_ptr<DoubleRegisterInterface> de,
    std::unique_ptr<DoubleRegisterInterface> hl,
    std::unique_ptr<DoubleRegisterInterface> pc,
    std::unique_ptr<DoubleRegisterInterface> sp,
    memory::MemoryInterface *memory)
    : a(std::move(a)), b(std::move(b)), c(std::move(c)), d(std::move(d)),
      e(std::move(e)), f(std::move(f)), h(std::move(h)), l(std::move(l)),
      af(std::move(af)), bc(std::move(bc)), de(std::move(de)),
      hl(std::move(hl)), pc(std::move(pc)), sp(std::move(sp)), memory(memory),
      halted(false), interrupt_enabled(true), branch_taken(false),

      // Initialize the opcode map
      opcode_map({
          // clang-format off
          /* 0x00 */ [&] { op_nop(); },
          /* 0x01 */ [&] { op_ld_dbl(this->bc.get(), get_inst_dbl()); },
          /* 0x02 */ [&] { op_ld(this->bc->get(), this->a->get()); },
          /* 0x03 */ [&] { op_inc_dbl(this->bc.get()); },
          /* 0x04 */ [&] { op_inc(this->b.get()); },
          /* 0x05 */ [&] { op_dec(this->b.get()); },
          /* 0x06 */ [&] { op_ld(this->b.get(), get_inst_byte()); },
          /* 0x07 */ [&] { op_rlc_a(); },
          /* 0x08 */ [&] { op_ld_dbl(static_cast<Address>(get_inst_dbl()), this->sp->get()); },
          /* 0x09 */ [&] { op_add_hl(this->bc->get()); },
          /* 0x0a */ [&] { op_ld(this->a.get(), this->memory->read(this->bc->get())); },
          /* 0x0b */ [&] { op_dec_dbl(this->bc.get()); },
          /* 0x0c */ [&] { op_inc(this->c.get()); },
          /* 0x0d */ [&] { op_dec(this->c.get()); },
          /* 0x0e */ [&] { op_ld(this->c.get(), get_inst_byte()); },
          /* 0x0f */ [&] { op_rrc_a(); },
          /* 0x10 */ [&] { op_stop(); },
          /* 0x11 */ [&] { op_ld_dbl(this->de.get(), get_inst_dbl()); },
          /* 0x12 */ [&] { op_ld(this->de->get(), this->a->get()); },
          /* 0x13 */ [&] { op_inc_dbl(this->de.get()); },
          /* 0x14 */ [&] { op_inc(this->d.get()); },
          /* 0x15 */ [&] { op_dec(this->d.get()); },
          /* 0x16 */ [&] { op_ld(this->d.get(), get_inst_byte()); },
          /* 0x17 */ [&] { op_rl_a(); },
          /* 0x18 */ [&] { op_jr(get_inst_byte()); },
          /* 0x19 */ [&] { op_add_hl(this->de->get()); },
          /* 0x1a */ [&] { op_ld(this->a.get(), this->memory->read(this->de->get())); },
          /* 0x1b */ [&] { op_dec_dbl(this->de.get()); },
          /* 0x1c */ [&] { op_inc(this->e.get()); },
          /* 0x1d */ [&] { op_dec(this->e.get()); },
          /* 0x1e */ [&] { op_ld(this->e.get(), get_inst_byte()); },
          /* 0x1f */ [&] { op_rr_a(); },
          /* 0x20 */ [&] { op_jr(!this->f->get_bit(flag::ZERO), get_inst_byte()); },
          /* 0x21 */ [&] { op_ld_dbl(this->hl.get(), get_inst_dbl()); },
          /* 0x22 */ [&] { op_ldi_addr(this->hl->get(), this->a->get()); },
          /* 0x23 */ [&] { op_inc_dbl(this->hl.get()); },
          /* 0x24 */ [&] { op_inc(this->h.get()); },
          /* 0x25 */ [&] { op_dec(this->h.get()); },
          /* 0x26 */ [&] { op_ld(this->h.get(), get_inst_byte()); },
          /* 0x27 */ [&] { op_daa(); },
          /* 0x28 */ [&] { op_jr(this->f->get_bit(flag::ZERO), get_inst_byte()); },
          /* 0x29 */ [&] { op_add_hl(this->hl->get()); },
          /* 0x2a */ [&] { op_ldi_a(this->memory->read(this->hl->get())); },
          /* 0x2b */ [&] { op_dec_dbl(this->hl.get()); },
          /* 0x2c */ [&] { op_inc(this->l.get()); },
          /* 0x2d */ [&] { op_dec(this->l.get()); },
          /* 0x2e */ [&] { op_ld(this->l.get(), get_inst_byte()); },
          /* 0x2f */ [&] { op_cpl(); },
          /* 0x30 */ [&] { op_jr(!this->f->get_bit(flag::CARRY), get_inst_byte()); },
          /* 0x31 */ [&] { op_ld_dbl(this->sp.get(), get_inst_dbl()); },
          /* 0x32 */ [&] { op_ldd_addr(static_cast<Address>(this->hl->get()), this->a->get()); },
          /* 0x33 */ [&] { op_inc_dbl(this->sp.get()); },
          /* 0x34 */ [&] { op_inc(static_cast<Address>(this->hl->get())); },
          /* 0x35 */ [&] { op_dec(static_cast<Address>(this->hl->get())); },
          /* 0x36 */ [&] { op_ld(static_cast<Address>(this->hl->get()), get_inst_byte()); },
          /* 0x37 */ [&] { op_scf(); },
          /* 0x38 */ [&] { op_jr(this->f->get_bit(flag::CARRY), get_inst_byte()); },
          /* 0x39 */ [&] { op_add_hl(this->sp->get()); },
          /* 0x3a */ [&] { op_ldd_a(this->memory->read(this->hl->get())); },
          /* 0x3b */ [&] { op_dec_dbl(this->sp.get()); },
          /* 0x3c */ [&] { op_inc(this->a.get()); },
          /* 0x3d */ [&] { op_dec(this->a.get()); },
          /* 0x3e */ [&] { op_ld(this->a.get(), get_inst_byte()); },
          /* 0x3f */ [&] { op_ccf(); },
          /* 0x40 */ [&] { op_ld(this->b.get(), this->b->get()); },
          /* 0x41 */ [&] { op_ld(this->b.get(), this->c->get()); },
          /* 0x42 */ [&] { op_ld(this->b.get(), this->d->get()); },
          /* 0x43 */ [&] { op_ld(this->b.get(), this->e->get()); },
          /* 0x44 */ [&] { op_ld(this->b.get(), this->h->get()); },
          /* 0x45 */ [&] { op_ld(this->b.get(), this->l->get()); },
          /* 0x46 */ [&] { op_ld(this->b.get(), this->memory->read(this->hl->get())); },
          /* 0x47 */ [&] { op_ld(this->b.get(), this->a->get()); },
          /* 0x48 */ [&] { op_ld(this->c.get(), this->b->get()); },
          /* 0x49 */ [&] { op_ld(this->c.get(), this->c->get()); },
          /* 0x4a */ [&] { op_ld(this->c.get(), this->d->get()); },
          /* 0x4b */ [&] { op_ld(this->c.get(), this->e->get()); },
          /* 0x4c */ [&] { op_ld(this->c.get(), this->h->get()); },
          /* 0x4d */ [&] { op_ld(this->c.get(), this->l->get()); },
          /* 0x4e */ [&] { op_ld(this->c.get(), this->memory->read(this->hl->get())); },
          /* 0x4f */ [&] { op_ld(this->c.get(), this->a->get()); },
          /* 0x50 */ [&] { op_ld(this->d.get(), this->b->get()); },
          /* 0x51 */ [&] { op_ld(this->d.get(), this->c->get()); },
          /* 0x52 */ [&] { op_ld(this->d.get(), this->d->get()); },
          /* 0x53 */ [&] { op_ld(this->d.get(), this->e->get()); },
          /* 0x54 */ [&] { op_ld(this->d.get(), this->h->get()); },
          /* 0x55 */ [&] { op_ld(this->d.get(), this->l->get()); },
          /* 0x56 */ [&] { op_ld(this->d.get(), this->memory->read(this->hl->get())); },
          /* 0x57 */ [&] { op_ld(this->d.get(), this->a->get()); },
          /* 0x58 */ [&] { op_ld(this->e.get(), this->b->get()); },
          /* 0x59 */ [&] { op_ld(this->e.get(), this->c->get()); },
          /* 0x5a */ [&] { op_ld(this->e.get(), this->d->get()); },
          /* 0x5b */ [&] { op_ld(this->e.get(), this->e->get()); },
          /* 0x5c */ [&] { op_ld(this->e.get(), this->h->get()); },
          /* 0x5d */ [&] { op_ld(this->e.get(), this->l->get()); },
          /* 0x5e */ [&] { op_ld(this->e.get(), this->memory->read(this->hl->get())); },
          /* 0x5f */ [&] { op_ld(this->e.get(), this->a->get()); },
          /* 0x60 */ [&] { op_ld(this->h.get(), this->b->get()); },
          /* 0x61 */ [&] { op_ld(this->h.get(), this->c->get()); },
          /* 0x62 */ [&] { op_ld(this->h.get(), this->d->get()); },
          /* 0x63 */ [&] { op_ld(this->h.get(), this->e->get()); },
          /* 0x64 */ [&] { op_ld(this->h.get(), this->h->get()); },
          /* 0x65 */ [&] { op_ld(this->h.get(), this->l->get()); },
          /* 0x66 */ [&] { op_ld(this->h.get(), this->memory->read(this->hl->get())); },
          /* 0x67 */ [&] { op_ld(this->h.get(), this->a->get()); },
          /* 0x68 */ [&] { op_ld(this->l.get(), this->b->get()); },
          /* 0x69 */ [&] { op_ld(this->l.get(), this->c->get()); },
          /* 0x6a */ [&] { op_ld(this->l.get(), this->d->get()); },
          /* 0x6b */ [&] { op_ld(this->l.get(), this->e->get()); },
          /* 0x6c */ [&] { op_ld(this->l.get(), this->h->get()); },
          /* 0x6d */ [&] { op_ld(this->l.get(), this->l->get()); },
          /* 0x6e */ [&] { op_ld(this->l.get(), this->memory->read(this->hl->get())); },
          /* 0x6f */ [&] { op_ld(this->l.get(), this->a->get()); },
          /* 0x70 */ [&] { op_ld(static_cast<Address>(this->hl->get()), this->b->get()); },
          /* 0x71 */ [&] { op_ld(static_cast<Address>(this->hl->get()), this->c->get()); },
          /* 0x72 */ [&] { op_ld(static_cast<Address>(this->hl->get()), this->d->get()); },
          /* 0x73 */ [&] { op_ld(static_cast<Address>(this->hl->get()), this->e->get()); },
          /* 0x74 */ [&] { op_ld(static_cast<Address>(this->hl->get()), this->h->get()); },
          /* 0x75 */ [&] { op_ld(static_cast<Address>(this->hl->get()), this->l->get()); },
          /* 0x76 */ [&] { op_halt(); },
          /* 0x77 */ [&] { op_ld(static_cast<Address>(this->hl->get()), this->a->get()); },
          /* 0x78 */ [&] { op_ld(this->a.get(), this->b->get()); },
          /* 0x79 */ [&] { op_ld(this->a.get(), this->c->get()); },
          /* 0x7a */ [&] { op_ld(this->a.get(), this->d->get()); },
          /* 0x7b */ [&] { op_ld(this->a.get(), this->e->get()); },
          /* 0x7c */ [&] { op_ld(this->a.get(), this->h->get()); },
          /* 0x7d */ [&] { op_ld(this->a.get(), this->l->get()); },
          /* 0x7e */ [&] { op_ld(this->a.get(), this->memory->read(this->hl->get())); },
          /* 0x7f */ [&] { op_ld(this->a.get(), this->a->get()); },
          /* 0x80 */ [&] { op_add(this->b->get()); },
          /* 0x81 */ [&] { op_add(this->c->get()); },
          /* 0x82 */ [&] { op_add(this->d->get()); },
          /* 0x83 */ [&] { op_add(this->e->get()); },
          /* 0x84 */ [&] { op_add(this->h->get()); },
          /* 0x85 */ [&] { op_add(this->l->get()); },
          /* 0x86 */ [&] { op_add(this->memory->read(this->hl->get())); },
          /* 0x87 */ [&] { op_add(this->a->get()); },
          /* 0x88 */ [&] { op_adc(this->b->get()); },
          /* 0x89 */ [&] { op_adc(this->c->get()); },
          /* 0x8a */ [&] { op_adc(this->d->get()); },
          /* 0x8b */ [&] { op_adc(this->e->get()); },
          /* 0x8c */ [&] { op_adc(this->h->get()); },
          /* 0x8d */ [&] { op_adc(this->l->get()); },
          /* 0x8e */ [&] { op_adc(this->memory->read(this->hl->get())); },
          /* 0x8f */ [&] { op_adc(this->a->get()); },
          /* 0x90 */ [&] { op_sub(this->b->get()); },
          /* 0x91 */ [&] { op_sub(this->c->get()); },
          /* 0x92 */ [&] { op_sub(this->d->get()); },
          /* 0x93 */ [&] { op_sub(this->e->get()); },
          /* 0x94 */ [&] { op_sub(this->h->get()); },
          /* 0x95 */ [&] { op_sub(this->l->get()); },
          /* 0x96 */ [&] { op_sub(this->memory->read(this->hl->get())); },
          /* 0x97 */ [&] { op_sub(this->a->get()); },
          /* 0x98 */ [&] { op_sbc(this->b->get()); },
          /* 0x99 */ [&] { op_sbc(this->c->get()); },
          /* 0x9a */ [&] { op_sbc(this->d->get()); },
          /* 0x9b */ [&] { op_sbc(this->e->get()); },
          /* 0x9c */ [&] { op_sbc(this->h->get()); },
          /* 0x9d */ [&] { op_sbc(this->l->get()); },
          /* 0x9e */ [&] { op_sbc(this->memory->read(this->hl->get())); },
          /* 0x9f */ [&] { op_sbc(this->a->get()); },
          /* 0xa0 */ [&] { op_and(this->b->get()); },
          /* 0xa1 */ [&] { op_and(this->c->get()); },
          /* 0xa2 */ [&] { op_and(this->d->get()); },
          /* 0xa3 */ [&] { op_and(this->e->get()); },
          /* 0xa4 */ [&] { op_and(this->h->get()); },
          /* 0xa5 */ [&] { op_and(this->l->get()); },
          /* 0xa6 */ [&] { op_and(this->memory->read(this->hl->get())); },
          /* 0xa7 */ [&] { op_and(this->a->get()); },
          /* 0xa8 */ [&] { op_xor(this->b->get()); },
          /* 0xa9 */ [&] { op_xor(this->c->get()); },
          /* 0xaa */ [&] { op_xor(this->d->get()); },
          /* 0xab */ [&] { op_xor(this->e->get()); },
          /* 0xac */ [&] { op_xor(this->h->get()); },
          /* 0xad */ [&] { op_xor(this->l->get()); },
          /* 0xae */ [&] { op_xor(this->memory->read(this->hl->get())); },
          /* 0xaf */ [&] { op_xor(this->a->get()); },
          /* 0xb0 */ [&] { op_or(this->b->get()); },
          /* 0xb1 */ [&] { op_or(this->c->get()); },
          /* 0xb2 */ [&] { op_or(this->d->get()); },
          /* 0xb3 */ [&] { op_or(this->e->get()); },
          /* 0xb4 */ [&] { op_or(this->h->get()); },
          /* 0xb5 */ [&] { op_or(this->l->get()); },
          /* 0xb6 */ [&] { op_or(this->memory->read(this->hl->get())); },
          /* 0xb7 */ [&] { op_or(this->a->get()); },
          /* 0xb8 */ [&] { op_cp(this->b->get()); },
          /* 0xb9 */ [&] { op_cp(this->c->get()); },
          /* 0xba */ [&] { op_cp(this->d->get()); },
          /* 0xbb */ [&] { op_cp(this->e->get()); },
          /* 0xbc */ [&] { op_cp(this->h->get()); },
          /* 0xbd */ [&] { op_cp(this->l->get()); },
          /* 0xbe */ [&] { op_cp(this->memory->read(this->hl->get())); },
          /* 0xbf */ [&] { op_cp(this->a->get()); },
          /* 0xc0 */ [&] { op_ret(!this->f->get_bit(flag::ZERO)); },
          /* 0xc1 */ [&] { op_pop(this->bc.get()); },
          /* 0xc2 */ [&] { op_jr(!this->f->get_bit(flag::ZERO), get_inst_dbl()); },
          /* 0xc3 */ [&] { op_jp(get_inst_dbl()); },
          /* 0xc4 */ [&] { op_call(!this->f->get_bit(flag::ZERO), get_inst_dbl()); },
          /* 0xc5 */ [&] { op_push(this->bc.get()); },
          /* 0xc6 */ [&] { op_add(get_inst_byte()); },
          /* 0xc7 */ [&] { op_rst(0x00); },
          /* 0xc8 */ [&] { op_ret(this->f->get_bit(flag::ZERO)); },
          /* 0xc9 */ [&] { op_ret(); },
          /* 0xca */ [&] { op_jr(this->f->get_bit(flag::ZERO), get_inst_dbl()); },
          /* 0xcb */ [&] { /* CB Opcodes handled separately */ },
          /* 0xcc */ [&] { op_call(this->f->get_bit(flag::ZERO), get_inst_dbl()); },
          /* 0xcd */ [&] { op_call(get_inst_dbl()); },
          /* 0xce */ [&] { op_adc(get_inst_byte()); },
          /* 0xcf */ [&] { op_rst(0x08); },
          /* 0xd0 */ [&] { op_ret(!this->f->get_bit(flag::CARRY)); },
          /* 0xd1 */ [&] { op_pop(this->de.get()); },
          /* 0xd2 */ [&] { op_jr(!this->f->get_bit(flag::CARRY), get_inst_dbl()); },
          /* 0xd3 */ [&] { /* UNDEFINED */ },
          /* 0xd4 */ [&] { op_call(!this->f->get_bit(flag::CARRY), get_inst_dbl()); },
          /* 0xd5 */ [&] { op_push(this->de.get()); },
          /* 0xd6 */ [&] { op_sub(get_inst_byte()); },
          /* 0xd7 */ [&] { op_rst(0x10); },
          /* 0xd8 */ [&] { op_ret(this->f->get_bit(flag::CARRY)); },
          /* 0xd9 */ [&] { op_reti(); },
          /* 0xda */ [&] { op_jr(this->f->get_bit(flag::CARRY), get_inst_dbl()); },
          /* 0xdb */ [&] { /* UNDEFINED */ },
          /* 0xdc */ [&] { op_call(this->f->get_bit(flag::CARRY), get_inst_dbl()); },
          /* 0xdd */ [&] { /* UNDEFINED */ },
          /* 0xde */ [&] { op_sbc(get_inst_byte()); },
          /* 0xdf */ [&] { op_rst(0x18); },
          /* 0xe0 */ [&] { op_ldh_addr(0xFF00 + get_inst_byte(), this->a->get()); },
          /* 0xe1 */ [&] { op_pop(this->hl.get()); },
          /* 0xe2 */ [&] { op_ld(static_cast<Address>(0xFF00 + this->c->get()), this->a->get()); },
          /* 0xe3 */ [&] { /* UNDEFINED */ },
          /* 0xe4 */ [&] { /* UNDEFINED */ },
          /* 0xe5 */ [&] { op_push(this->hl.get()); },
          /* 0xe6 */ [&] { op_and(get_inst_byte()); },
          /* 0xe7 */ [&] { op_rst(0x20); },
          /* 0xe8 */ [&] { op_add_sp(static_cast<int8_t>(get_inst_byte())); },
          /* 0xe9 */ [&] { op_jp(this->hl->get()); },
          /* 0xea */ [&] { op_ld_dbl(static_cast<Address>(get_inst_dbl()), this->a->get()); },
          /* 0xeb */ [&] { /* UNDEFINED */ },
          /* 0xec */ [&] { /* UNDEFINED */ },
          /* 0xed */ [&] { /* UNDEFINED */ },
          /* 0xee */ [&] { op_xor(get_inst_byte()); },
          /* 0xef */ [&] { op_rst(0x28); },
          /* 0xf0 */ [&] { op_ldh_a(0xFF00 + get_inst_byte()); },
          /* 0xf1 */ [&] { op_pop(this->af.get()); },
          /* 0xf2 */ [&] { op_ld(this->a.get(), this->memory->read(0xFF00 + this->c->get())); },
          /* 0xf3 */ [&] { op_di(); },
          /* 0xf4 */ [&] { /* UNDEFINED */ },
          /* 0xf5 */ [&] { op_push(this->af.get()); },
          /* 0xf6 */ [&] { op_or(get_inst_byte()); },
          /* 0xf7 */ [&] { op_rst(0x30); },
          /* 0xf8 */ [&] { op_ld_hl_sp_offset(static_cast<int8_t>(get_inst_byte())); },
          /* 0xf9 */ [&] { op_ld_dbl(this->sp.get(), this->hl->get()); },
          /* 0xfa */ [&] { op_ld(this->a.get(), this->memory->read(get_inst_dbl())); },
          /* 0xfb */ [&] { op_di(); },
          /* 0xfc */ [&] { /* UNDEFINED */ },
          /* 0xfd */ [&] { /* UNDEFINED */ },
          /* 0xfe */ [&] { op_cp(get_inst_byte()); },
          /* 0xff */ [&] { op_rst(0x38); },
          // clang-format on
      }),

      // Initialize the CB opcode map
      cb_opcode_map({
          // clang-format off
          /* 0x00 */ [&] { op_rlc(this->b.get()); },
          /* 0x01 */ [&] { op_rlc(this->c.get()); },
          /* 0x02 */ [&] { op_rlc(this->d.get()); },
          /* 0x03 */ [&] { op_rlc(this->e.get()); },
          /* 0x04 */ [&] { op_rlc(this->h.get()); },
          /* 0x05 */ [&] { op_rlc(this->l.get()); },
          /* 0x06 */ [&] { op_rlc(static_cast<Address>(this->hl->get())); },
          /* 0x07 */ [&] { op_rlc(this->a.get()); },
          /* 0x08 */ [&] { op_rrc(this->b.get()); },
          /* 0x09 */ [&] { op_rrc(this->c.get()); },
          /* 0x0a */ [&] { op_rrc(this->d.get()); },
          /* 0x0b */ [&] { op_rrc(this->e.get()); },
          /* 0x0c */ [&] { op_rrc(this->h.get()); },
          /* 0x0d */ [&] { op_rrc(this->l.get()); },
          /* 0x0e */ [&] { op_rrc(static_cast<Address>(this->hl->get())); },
          /* 0x0f */ [&] { op_rrc(this->a.get()); },
          /* 0x10 */ [&] { op_rl(this->b.get()); },
          /* 0x11 */ [&] { op_rl(this->c.get()); },
          /* 0x12 */ [&] { op_rl(this->d.get()); },
          /* 0x13 */ [&] { op_rl(this->e.get()); },
          /* 0x14 */ [&] { op_rl(this->h.get()); },
          /* 0x15 */ [&] { op_rl(this->l.get()); },
          /* 0x16 */ [&] { op_rl(static_cast<Address>(this->hl->get())); },
          /* 0x17 */ [&] { op_rl(this->a.get()); },
          /* 0x18 */ [&] { op_rr(this->b.get()); },
          /* 0x19 */ [&] { op_rr(this->c.get()); },
          /* 0x1a */ [&] { op_rr(this->d.get()); },
          /* 0x1b */ [&] { op_rr(this->e.get()); },
          /* 0x1c */ [&] { op_rr(this->h.get()); },
          /* 0x1d */ [&] { op_rr(this->l.get()); },
          /* 0x1e */ [&] { op_rr(static_cast<Address>(this->hl->get())); },
          /* 0x1f */ [&] { op_rr(this->a.get()); },
          /* 0x20 */ [&] { op_sla(this->b.get()); },
          /* 0x21 */ [&] { op_sla(this->c.get()); },
          /* 0x22 */ [&] { op_sla(this->d.get()); },
          /* 0x23 */ [&] { op_sla(this->e.get()); },
          /* 0x24 */ [&] { op_sla(this->h.get()); },
          /* 0x25 */ [&] { op_sla(this->l.get()); },
          /* 0x26 */ [&] { op_sla(static_cast<Address>(this->hl->get())); },
          /* 0x27 */ [&] { op_sla(this->a.get()); },
          /* 0x28 */ [&] { op_sra(this->b.get()); },
          /* 0x29 */ [&] { op_sra(this->c.get()); },
          /* 0x2a */ [&] { op_sra(this->d.get()); },
          /* 0x2b */ [&] { op_sra(this->e.get()); },
          /* 0x2c */ [&] { op_sra(this->h.get()); },
          /* 0x2d */ [&] { op_sra(this->l.get()); },
          /* 0x2e */ [&] { op_sra(static_cast<Address>(this->hl->get())); },
          /* 0x2f */ [&] { op_sra(this->a.get()); },
          /* 0x30 */ [&] { op_swap(this->b.get()); },
          /* 0x31 */ [&] { op_swap(this->c.get()); },
          /* 0x32 */ [&] { op_swap(this->d.get()); },
          /* 0x33 */ [&] { op_swap(this->e.get()); },
          /* 0x34 */ [&] { op_swap(this->h.get()); },
          /* 0x35 */ [&] { op_swap(this->l.get()); },
          /* 0x36 */ [&] { op_swap(static_cast<Address>(this->hl->get())); },
          /* 0x37 */ [&] { op_swap(this->a.get()); },
          /* 0x38 */ [&] { op_srl(this->b.get()); },
          /* 0x39 */ [&] { op_srl(this->c.get()); },
          /* 0x3a */ [&] { op_srl(this->d.get()); },
          /* 0x3b */ [&] { op_srl(this->e.get()); },
          /* 0x3c */ [&] { op_srl(this->h.get()); },
          /* 0x3d */ [&] { op_srl(this->l.get()); },
          /* 0x3e */ [&] { op_srl(static_cast<Address>(this->hl->get())); },
          /* 0x3f */ [&] { op_srl(this->a.get()); },
          /* 0x40 */ [&] { op_bit(this->b.get(), 0); },
          /* 0x41 */ [&] { op_bit(this->c.get(), 0); },
          /* 0x42 */ [&] { op_bit(this->d.get(), 0); },
          /* 0x43 */ [&] { op_bit(this->e.get(), 0); },
          /* 0x44 */ [&] { op_bit(this->h.get(), 0); },
          /* 0x45 */ [&] { op_bit(this->l.get(), 0); },
          /* 0x46 */ [&] { op_bit(this->memory->read(this->hl->get()), 0); },
          /* 0x47 */ [&] { op_bit(this->a.get(), 0); },
          /* 0x48 */ [&] { op_bit(this->b.get(), 1); },
          /* 0x49 */ [&] { op_bit(this->c.get(), 1); },
          /* 0x4a */ [&] { op_bit(this->d.get(), 1); },
          /* 0x4b */ [&] { op_bit(this->e.get(), 1); },
          /* 0x4c */ [&] { op_bit(this->h.get(), 1); },
          /* 0x4d */ [&] { op_bit(this->l.get(), 1); },
          /* 0x4e */ [&] { op_bit(this->memory->read(this->hl->get()), 1); },
          /* 0x4f */ [&] { op_bit(this->a.get(), 1); },
          /* 0x50 */ [&] { op_bit(this->b.get(), 2); },
          /* 0x51 */ [&] { op_bit(this->c.get(), 2); },
          /* 0x52 */ [&] { op_bit(this->d.get(), 2); },
          /* 0x53 */ [&] { op_bit(this->e.get(), 2); },
          /* 0x54 */ [&] { op_bit(this->h.get(), 2); },
          /* 0x55 */ [&] { op_bit(this->l.get(), 2); },
          /* 0x56 */ [&] { op_bit(this->memory->read(this->hl->get()), 2); },
          /* 0x57 */ [&] { op_bit(this->a.get(), 2); },
          /* 0x58 */ [&] { op_bit(this->b.get(), 3); },
          /* 0x59 */ [&] { op_bit(this->c.get(), 3); },
          /* 0x5a */ [&] { op_bit(this->d.get(), 3); },
          /* 0x5b */ [&] { op_bit(this->e.get(), 3); },
          /* 0x5c */ [&] { op_bit(this->h.get(), 3); },
          /* 0x5d */ [&] { op_bit(this->l.get(), 3); },
          /* 0x5e */ [&] { op_bit(this->memory->read(this->hl->get()), 3); },
          /* 0x5f */ [&] { op_bit(this->a.get(), 3); },
          /* 0x60 */ [&] { op_bit(this->b.get(), 4); },
          /* 0x61 */ [&] { op_bit(this->c.get(), 4); },
          /* 0x62 */ [&] { op_bit(this->d.get(), 4); },
          /* 0x63 */ [&] { op_bit(this->e.get(), 4); },
          /* 0x64 */ [&] { op_bit(this->h.get(), 4); },
          /* 0x65 */ [&] { op_bit(this->l.get(), 4); },
          /* 0x66 */ [&] { op_bit(this->memory->read(this->hl->get()), 4); },
          /* 0x67 */ [&] { op_bit(this->a.get(), 4); },
          /* 0x68 */ [&] { op_bit(this->b.get(), 5); },
          /* 0x69 */ [&] { op_bit(this->c.get(), 5); },
          /* 0x6a */ [&] { op_bit(this->d.get(), 5); },
          /* 0x6b */ [&] { op_bit(this->e.get(), 5); },
          /* 0x6c */ [&] { op_bit(this->h.get(), 5); },
          /* 0x6d */ [&] { op_bit(this->l.get(), 5); },
          /* 0x6e */ [&] { op_bit(this->memory->read(this->hl->get()), 5); },
          /* 0x6f */ [&] { op_bit(this->a.get(), 5); },
          /* 0x70 */ [&] { op_bit(this->b.get(), 6); },
          /* 0x71 */ [&] { op_bit(this->c.get(), 6); },
          /* 0x72 */ [&] { op_bit(this->d.get(), 6); },
          /* 0x73 */ [&] { op_bit(this->e.get(), 6); },
          /* 0x74 */ [&] { op_bit(this->h.get(), 6); },
          /* 0x75 */ [&] { op_bit(this->l.get(), 6); },
          /* 0x76 */ [&] { op_bit(this->memory->read(this->hl->get()), 6); },
          /* 0x77 */ [&] { op_bit(this->a.get(), 6); },
          /* 0x78 */ [&] { op_bit(this->b.get(), 7); },
          /* 0x79 */ [&] { op_bit(this->c.get(), 7); },
          /* 0x7a */ [&] { op_bit(this->d.get(), 7); },
          /* 0x7b */ [&] { op_bit(this->e.get(), 7); },
          /* 0x7c */ [&] { op_bit(this->h.get(), 7); },
          /* 0x7d */ [&] { op_bit(this->l.get(), 7); },
          /* 0x7e */ [&] { op_bit(this->memory->read(this->hl->get()), 7); },
          /* 0x7f */ [&] { op_bit(this->a.get(), 7); },
          /* 0x80 */ [&] { op_res(this->b.get(), 0); },
          /* 0x81 */ [&] { op_res(this->c.get(), 0); },
          /* 0x82 */ [&] { op_res(this->d.get(), 0); },
          /* 0x83 */ [&] { op_res(this->e.get(), 0); },
          /* 0x84 */ [&] { op_res(this->h.get(), 0); },
          /* 0x85 */ [&] { op_res(this->l.get(), 0); },
          /* 0x86 */ [&] { op_res(this->hl->get(), 0); },
          /* 0x87 */ [&] { op_res(this->a.get(), 0); },
          /* 0x88 */ [&] { op_res(this->b.get(), 1); },
          /* 0x89 */ [&] { op_res(this->c.get(), 1); },
          /* 0x8a */ [&] { op_res(this->d.get(), 1); },
          /* 0x8b */ [&] { op_res(this->e.get(), 1); },
          /* 0x8c */ [&] { op_res(this->h.get(), 1); },
          /* 0x8d */ [&] { op_res(this->l.get(), 1); },
          /* 0x8e */ [&] { op_res(this->hl->get(), 1); },
          /* 0x8f */ [&] { op_res(this->a.get(), 1); },
          /* 0x90 */ [&] { op_res(this->b.get(), 2); },
          /* 0x91 */ [&] { op_res(this->c.get(), 2); },
          /* 0x92 */ [&] { op_res(this->d.get(), 2); },
          /* 0x93 */ [&] { op_res(this->e.get(), 2); },
          /* 0x94 */ [&] { op_res(this->h.get(), 2); },
          /* 0x95 */ [&] { op_res(this->l.get(), 2); },
          /* 0x96 */ [&] { op_res(this->hl->get(), 2); },
          /* 0x97 */ [&] { op_res(this->a.get(), 2); },
          /* 0x98 */ [&] { op_res(this->b.get(), 3); },
          /* 0x99 */ [&] { op_res(this->c.get(), 3); },
          /* 0x9a */ [&] { op_res(this->d.get(), 3); },
          /* 0x9b */ [&] { op_res(this->e.get(), 3); },
          /* 0x9c */ [&] { op_res(this->h.get(), 3); },
          /* 0x9d */ [&] { op_res(this->l.get(), 3); },
          /* 0x9e */ [&] { op_res(this->hl->get(), 3); },
          /* 0x9f */ [&] { op_res(this->a.get(), 3); },
          /* 0xa0 */ [&] { op_res(this->b.get(), 4); },
          /* 0xa1 */ [&] { op_res(this->c.get(), 4); },
          /* 0xa2 */ [&] { op_res(this->d.get(), 4); },
          /* 0xa3 */ [&] { op_res(this->e.get(), 4); },
          /* 0xa4 */ [&] { op_res(this->h.get(), 4); },
          /* 0xa5 */ [&] { op_res(this->l.get(), 4); },
          /* 0xa6 */ [&] { op_res(this->hl->get(), 4); },
          /* 0xa7 */ [&] { op_res(this->a.get(), 4); },
          /* 0xa8 */ [&] { op_res(this->b.get(), 5); },
          /* 0xa9 */ [&] { op_res(this->c.get(), 5); },
          /* 0xaa */ [&] { op_res(this->d.get(), 5); },
          /* 0xab */ [&] { op_res(this->e.get(), 5); },
          /* 0xac */ [&] { op_res(this->h.get(), 5); },
          /* 0xad */ [&] { op_res(this->l.get(), 5); },
          /* 0xae */ [&] { op_res(this->hl->get(), 5); },
          /* 0xaf */ [&] { op_res(this->a.get(), 5); },
          /* 0xb0 */ [&] { op_res(this->b.get(), 6); },
          /* 0xb1 */ [&] { op_res(this->c.get(), 6); },
          /* 0xb2 */ [&] { op_res(this->d.get(), 6); },
          /* 0xb3 */ [&] { op_res(this->e.get(), 6); },
          /* 0xb4 */ [&] { op_res(this->h.get(), 6); },
          /* 0xb5 */ [&] { op_res(this->l.get(), 6); },
          /* 0xb6 */ [&] { op_res(this->hl->get(), 6); },
          /* 0xb7 */ [&] { op_res(this->a.get(), 6); },
          /* 0xb8 */ [&] { op_res(this->b.get(), 7); },
          /* 0xb9 */ [&] { op_res(this->c.get(), 7); },
          /* 0xba */ [&] { op_res(this->d.get(), 7); },
          /* 0xbb */ [&] { op_res(this->e.get(), 7); },
          /* 0xbc */ [&] { op_res(this->h.get(), 7); },
          /* 0xbd */ [&] { op_res(this->l.get(), 7); },
          /* 0xbe */ [&] { op_res(this->hl->get(), 7); },
          /* 0xbf */ [&] { op_res(this->a.get(), 7); },
          /* 0xc0 */ [&] { op_set(this->b.get(), 0); },
          /* 0xc1 */ [&] { op_set(this->c.get(), 0); },
          /* 0xc2 */ [&] { op_set(this->d.get(), 0); },
          /* 0xc3 */ [&] { op_set(this->e.get(), 0); },
          /* 0xc4 */ [&] { op_set(this->h.get(), 0); },
          /* 0xc5 */ [&] { op_set(this->l.get(), 0); },
          /* 0xc6 */ [&] { op_set(this->hl->get(), 0); },
          /* 0xc7 */ [&] { op_set(this->a.get(), 0); },
          /* 0xc8 */ [&] { op_set(this->b.get(), 1); },
          /* 0xc9 */ [&] { op_set(this->c.get(), 1); },
          /* 0xca */ [&] { op_set(this->d.get(), 1); },
          /* 0xcb */ [&] { op_set(this->e.get(), 1); },
          /* 0xcc */ [&] { op_set(this->h.get(), 1); },
          /* 0xcd */ [&] { op_set(this->l.get(), 1); },
          /* 0xce */ [&] { op_set(this->hl->get(), 1); },
          /* 0xcf */ [&] { op_set(this->a.get(), 1); },
          /* 0xd0 */ [&] { op_set(this->b.get(), 2); },
          /* 0xd1 */ [&] { op_set(this->c.get(), 2); },
          /* 0xd2 */ [&] { op_set(this->d.get(), 2); },
          /* 0xd3 */ [&] { op_set(this->e.get(), 2); },
          /* 0xd4 */ [&] { op_set(this->h.get(), 2); },
          /* 0xd5 */ [&] { op_set(this->l.get(), 2); },
          /* 0xd6 */ [&] { op_set(this->hl->get(), 2); },
          /* 0xd7 */ [&] { op_set(this->a.get(), 2); },
          /* 0xd8 */ [&] { op_set(this->b.get(), 3); },
          /* 0xd9 */ [&] { op_set(this->c.get(), 3); },
          /* 0xda */ [&] { op_set(this->d.get(), 3); },
          /* 0xdb */ [&] { op_set(this->e.get(), 3); },
          /* 0xdc */ [&] { op_set(this->h.get(), 3); },
          /* 0xdd */ [&] { op_set(this->l.get(), 3); },
          /* 0xde */ [&] { op_set(this->hl->get(), 3); },
          /* 0xdf */ [&] { op_set(this->a.get(), 3); },
          /* 0xe0 */ [&] { op_set(this->b.get(), 4); },
          /* 0xe1 */ [&] { op_set(this->c.get(), 4); },
          /* 0xe2 */ [&] { op_set(this->d.get(), 4); },
          /* 0xe3 */ [&] { op_set(this->e.get(), 4); },
          /* 0xe4 */ [&] { op_set(this->h.get(), 4); },
          /* 0xe5 */ [&] { op_set(this->l.get(), 4); },
          /* 0xe6 */ [&] { op_set(this->hl->get(), 4); },
          /* 0xe7 */ [&] { op_set(this->a.get(), 4); },
          /* 0xe8 */ [&] { op_set(this->b.get(), 5); },
          /* 0xe9 */ [&] { op_set(this->c.get(), 5); },
          /* 0xea */ [&] { op_set(this->d.get(), 5); },
          /* 0xeb */ [&] { op_set(this->e.get(), 5); },
          /* 0xec */ [&] { op_set(this->h.get(), 5); },
          /* 0xed */ [&] { op_set(this->l.get(), 5); },
          /* 0xee */ [&] { op_set(this->hl->get(), 5); },
          /* 0xef */ [&] { op_set(this->a.get(), 5); },
          /* 0xf0 */ [&] { op_set(this->b.get(), 6); },
          /* 0xf1 */ [&] { op_set(this->c.get(), 6); },
          /* 0xf2 */ [&] { op_set(this->d.get(), 6); },
          /* 0xf3 */ [&] { op_set(this->e.get(), 6); },
          /* 0xf4 */ [&] { op_set(this->h.get(), 6); },
          /* 0xf5 */ [&] { op_set(this->l.get(), 6); },
          /* 0xf6 */ [&] { op_set(this->hl->get(), 6); },
          /* 0xf7 */ [&] { op_set(this->a.get(), 6); },
          /* 0xf8 */ [&] { op_set(this->b.get(), 7); },
          /* 0xf9 */ [&] { op_set(this->c.get(), 7); },
          /* 0xfa */ [&] { op_set(this->d.get(), 7); },
          /* 0xfb */ [&] { op_set(this->e.get(), 7); },
          /* 0xfc */ [&] { op_set(this->h.get(), 7); },
          /* 0xfd */ [&] { op_set(this->l.get(), 7); },
          /* 0xfe */ [&] { op_set(this->hl->get(), 7); },
          /* 0xff */ [&] { op_set(this->a.get(), 7); },
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

ClockCycles CPU::tick() {
	log_registers();
	auto opcode = get_inst_byte();
	Log::info("Program Counter : " + num_to_hex(pc->get()));
	auto current_cycles = ClockCycles{0};
	if (opcode != 0xCB) {
		Log::info("Current Opcode  : " + num_to_hex(opcode) + " -> " +
		          get_mnemonic(opcode));
		opcode_map[opcode]();
		current_cycles = cycles[opcode];
	} else {
		opcode = get_inst_byte();
		Log::info("Current Opcode  : " + num_to_hex(opcode) + " -> " +
		          get_cb_mnemonic(opcode));
		cb_opcode_map[opcode]();
		current_cycles = cycles_cb[opcode];
	}
	Log::info("Inst " + num_to_hex(opcode) + " done, took " +
	          std::to_string(current_cycles) + " cycles...");

	return current_cycles;
}

ClockCycles CPU::execute(uint8_t opcode, uint16_t pc) { return 0; }

uint8_t CPU::get_inst_byte() const {
	auto byte = memory->read(pc->get());
	(*pc)++;
	return byte;
};

uint16_t CPU::get_inst_dbl() const {
	auto lower = get_inst_byte();
	auto upper = get_inst_byte();

	auto result = static_cast<uint16_t>((upper << 8) | lower);
	return result;
};

void CPU::log_registers() {
	Log::info("A  -> " + num_to_hex(a->get()) + " | F  -> " +
	          num_to_hex(f->get()));
	Log::info("B  -> " + num_to_hex(b->get()) + " | C  -> " +
	          num_to_hex(c->get()));
	Log::info("D  -> " + num_to_hex(d->get()) + " | E  -> " +
	          num_to_hex(e->get()));
	Log::info("H  -> " + num_to_hex(h->get()) + " | L  -> " +
	          num_to_hex(l->get()));
	Log::info("PC -> " + num_to_hex(pc->get()));
	Log::info("SP -> " + num_to_hex(sp->get()));
}

} // namespace cpu
