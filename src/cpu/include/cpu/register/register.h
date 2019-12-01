/**
 * @file register.h
 * Declares the Register Classes
 */

#include "cpu/register/register_interface.h"
#include "cpu/utils.h"
#include "util/helpers.h"

#include <cstdint>

#pragma once

namespace cpu {

/**
 * An 8-bit register class, that holds a value and can perform
 * bit operations on it
 */
class Register : public RegisterInterface {
	/**
	 * 8-bit value stored in this register
	 */
	uint8_t _value;

  public:
	Register();
	Register(uint8_t value);

	/**
	 * @see RegisterInterface#set
	 */
	void set(uint8_t value) override;

	/**
	 * @see RegisterInterface#get
	 */
	uint8_t get() const override;

	/**
	 * @see RegisterInterface#set_bit
	 */
	void set_bit(uint8_t bit, bool value) override;

	/**
	 * @see RegisterInterface#get_bit
	 */
	bool get_bit(uint8_t bit) const override;

	/**
	 * @see RegisterInterface#operator++
	 */
	void operator++() override;

	/**
	 * @see RegisterInterface#operator++
	 */
	void operator++(int) override;

	/**
	 * @see RegisterInterface#operator--
	 */
	void operator--() override;

	/**
	 * @see RegisterInterface#operator--
	 */
	void operator--(int) override;
};

/**
 * An 16-bit register class, that holds a value and can perform
 * bit operations on it. Used for the larger registers like SP and PC
 */
class DoubleRegister : public DoubleRegisterInterface {
	/**
	 * 8-bit value stored in this register
	 */
	uint16_t _value;

  public:
	DoubleRegister();
	DoubleRegister(uint16_t value);

	/**
	 * @see DoubleRegisterInterface#set
	 */
	void set(uint16_t value) override;

	/**
	 * @see DoubleRegisterInterface#get
	 */
	uint16_t get() const override;

	/**
	 * @see DoubleRegisterInterface#set_bit
	 */
	void set_bit(uint8_t bit, bool value) override;

	/**
	 * @see DoubleRegisterInterface#get_bit
	 */
	bool get_bit(uint8_t bit) const override;

	/**
	 * @see DoubleRegisterInterface#get_high
	 */
	uint8_t get_high() const override;

	/**
	 * @see DoubleRegisterInterface#get_low
	 */
	uint8_t get_low() const override;

	/**
	 * @see DoubleRegisterInterface#operator++
	 */
	void operator++() override;

	/**
	 * @see DoubleRegisterInterface#operator++
	 */
	void operator++(int) override;

	/**
	 * @see DoubleRegisterInterface#operator--
	 */
	void operator--() override;

	/**
	 * @see DoubleRegisterInterface#operator--
	 */
	void operator--(int) override;
};

/**
 * A 16-bit register class, that aggregates and performs operations on two
 * existing 8-bit registers.
 *
 * For example, Registers B and C can act together as a 16-bit BC register
 */
class PairRegister : public DoubleRegisterInterface {
	RegisterInterface *_first;
	RegisterInterface *_second;

  public:
	PairRegister(RegisterInterface *first, RegisterInterface *second);

	/**
	 * @see DoubleRegisterInterface#set
	 */
	void set(uint16_t value) override;

	/**
	 * @see DoubleRegisterInterface#get
	 */
	uint16_t get() const override;

	/**
	 * @see DoubleRegisterInterface#set_bit
	 */
	void set_bit(uint8_t bit, bool value) override;

	/**
	 * @see DoubleRegisterInterface#get_bit
	 */
	bool get_bit(uint8_t bit) const override;

	/**
	 * @see DoubleRegisterInterface#get_high
	 */
	uint8_t get_high() const override;

	/**
	 * @see DoubleRegisterInterface#get_low
	 */
	uint8_t get_low() const override;

	/**
	 * @see DoubleRegisterInterface#operator++
	 */
	void operator++() override;

	/**
	 * @see DoubleRegisterInterface#operator++
	 */
	void operator++(int) override;

	/**
	 * @see DoubleRegisterInterface#operator--
	 */
	void operator--() override;

	/**
	 * @see DoubleRegisterInterface#operator--
	 */
	void operator--(int) override;
};

} // namespace cpu
