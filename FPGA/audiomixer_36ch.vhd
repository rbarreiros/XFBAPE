-- 36-channel Audiomixer
-- (c) 2023-2024 Dr.-Ing. Christian Noeding
-- christian@noeding-online.de
-- Released under GNU General Public License v3
-- Source: https://www.github.com/xn--nding-jua/xfbape
--
-- This file contains a multi-channel audiomixer

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity audiomixer_36ch is
   generic (
      bit_width		:	natural range 16 to 48 := 24
   );
	port (
		clk				:	in std_logic := '0';
		ch1_in			:	in signed(23 downto 0) := (others=>'0');
		ch2_in			:	in signed(23 downto 0) := (others=>'0');
		ch3_in			:	in signed(23 downto 0) := (others=>'0');
		ch4_in			:	in signed(23 downto 0) := (others=>'0');
		ch5_in			:	in signed(23 downto 0) := (others=>'0');
		ch6_in			:	in signed(23 downto 0) := (others=>'0');
		ch7_in			:	in signed(23 downto 0) := (others=>'0');
		ch8_in			:	in signed(23 downto 0) := (others=>'0');
		ch9_in			:	in signed(23 downto 0) := (others=>'0');
		ch10_in			:	in signed(23 downto 0) := (others=>'0');
		ch11_in			:	in signed(23 downto 0) := (others=>'0');
		ch12_in			:	in signed(23 downto 0) := (others=>'0');
		ch13_in			:	in signed(23 downto 0) := (others=>'0');
		ch14_in			:	in signed(23 downto 0) := (others=>'0');
		ch15_in			:	in signed(23 downto 0) := (others=>'0');
		ch16_in			:	in signed(23 downto 0) := (others=>'0');
		ch17_in			:	in signed(23 downto 0) := (others=>'0');
		ch18_in			:	in signed(23 downto 0) := (others=>'0');
		ch19_in			:	in signed(23 downto 0) := (others=>'0');
		ch20_in			:	in signed(23 downto 0) := (others=>'0');
		ch21_in			:	in signed(23 downto 0) := (others=>'0');
		ch22_in			:	in signed(23 downto 0) := (others=>'0');
		ch23_in			:	in signed(23 downto 0) := (others=>'0');
		ch24_in			:	in signed(23 downto 0) := (others=>'0');
		ch25_in			:	in signed(23 downto 0) := (others=>'0');
		ch26_in			:	in signed(23 downto 0) := (others=>'0');
		ch27_in			:	in signed(23 downto 0) := (others=>'0');
		ch28_in			:	in signed(23 downto 0) := (others=>'0');
		ch29_in			:	in signed(23 downto 0) := (others=>'0');
		ch30_in			:	in signed(23 downto 0) := (others=>'0');
		ch31_in			:	in signed(23 downto 0) := (others=>'0');
		ch32_in			:	in signed(23 downto 0) := (others=>'0');
		ch33_in			:	in signed(23 downto 0) := (others=>'0');
		ch34_in			:	in signed(23 downto 0) := (others=>'0');
		ch35_in			:	in signed(23 downto 0) := (others=>'0');
		ch36_in			:	in signed(23 downto 0) := (others=>'0');
		sync_in			:	in std_logic := '0';

		vol_ch1_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch1_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch2_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch2_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch3_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch3_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch4_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch4_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch5_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch5_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch6_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch6_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch7_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch7_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch8_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch8_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch9_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch9_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch10_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch10_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch11_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch11_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch12_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch12_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch13_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch13_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch14_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch14_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch15_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch15_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch16_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch16_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch17_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch17_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch18_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch18_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch19_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch19_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch20_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch20_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch21_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch21_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch22_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch22_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch23_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch23_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch24_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch24_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch25_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch25_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch26_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch26_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch27_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch27_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch28_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch28_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch29_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch29_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch30_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch30_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch31_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch31_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch32_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch32_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch33_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch33_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch34_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch34_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch35_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch35_r		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch36_l		:	in std_logic_vector(7 downto 0) := (others=>'0');
		vol_ch36_r		:	in std_logic_vector(7 downto 0) := (others=>'0');

		main_l_out		:	out signed(bit_width - 1 downto 0) := (others=>'0'); -- use a specific number of additional bits to prevent signal-clipping
		main_r_out		:	out signed(bit_width - 1 downto 0) := (others=>'0'); -- use a specific number of additional bits to prevent signal-clipping
		sync_out			:	out std_logic := '0'
		);
end audiomixer_36ch;

architecture Behavioral of audiomixer_36ch is
	signal state		:	natural range 0 to 19 := 0;

	-- signals for multipliers
	signal mult_in_sample	:	signed(23 downto 0) := (others=>'0');
	signal mult_in_vol_l		:	signed(8 downto 0) := (others=>'0');
	signal mult_in_vol_r		:	signed(8 downto 0) := (others=>'0');
	signal mult_out_l			:	signed(32 downto 0) := (others=>'0');
	signal mult_out_r			:	signed(32 downto 0) := (others=>'0');
	
	signal mult2_in_sample	:	signed(23 downto 0) := (others=>'0');
	signal mult2_in_vol_l	:	signed(8 downto 0) := (others=>'0');
	signal mult2_in_vol_r	:	signed(8 downto 0) := (others=>'0');
	signal mult2_out_l		:	signed(32 downto 0) := (others=>'0');
	signal mult2_out_r		:	signed(32 downto 0) := (others=>'0');

	signal main_l 				:	signed(bit_width - 1 downto 0);
	signal main_r 				:	signed(bit_width - 1 downto 0);
begin
	-- multiplier for left channel
	process(mult_in_sample, mult_in_vol_l)
	begin
		mult_out_l <= mult_in_sample * mult_in_vol_l;
	end process;
	-- multiplier for right channel
	process(mult_in_sample, mult_in_vol_r)
	begin
		mult_out_r <= mult_in_sample * mult_in_vol_r;
	end process;

	-- multiplier 2 for left channel
	process(mult2_in_sample, mult2_in_vol_l)
	begin
		mult2_out_l <= mult2_in_sample * mult2_in_vol_l;
	end process;
	-- multiplier 2 for right channel
	process(mult2_in_sample, mult2_in_vol_r)
	begin
		mult2_out_r <= mult2_in_sample * mult2_in_vol_r;
	end process;
	
	-- main-process
	process(clk)
	begin
		if rising_edge(clk) then
			if (sync_in = '1' and state = 0) then
				-- load multiplicator 1
				mult_in_sample <= ch1_in;
				mult_in_vol_l <= signed("0" & vol_ch1_l);
				mult_in_vol_r <= signed("0" & vol_ch1_r);

				-- load multiplicator 2
				mult2_in_sample <= ch2_in;
				mult2_in_vol_l <= signed("0" & vol_ch2_l);
				mult2_in_vol_r <= signed("0" & vol_ch2_r);

				state <= 1;
			elsif (state = 1) then
				-- now we have to divide the multiplicated signal by 128.
				-- We can bitshift this by 7 to the right
				main_l <= resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				-- load multiplicator 1 with next channel
				mult_in_sample <= ch3_in;
				mult_in_vol_l <= signed("0" & vol_ch3_l);
				mult_in_vol_r <= signed("0" & vol_ch3_r);

				-- load multiplicator 2 with next channel
				mult2_in_sample <= ch4_in;
				mult2_in_vol_l <= signed("0" & vol_ch4_l);
				mult2_in_vol_r <= signed("0" & vol_ch4_r);

				state <= state + 1;
			elsif (state = 2) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch5_in;
				mult_in_vol_l <= signed("0" & vol_ch5_l);
				mult_in_vol_r <= signed("0" & vol_ch5_r);

				mult2_in_sample <= ch6_in;
				mult2_in_vol_l <= signed("0" & vol_ch6_l);
				mult2_in_vol_r <= signed("0" & vol_ch6_r);
				
				state <= state + 1;
			elsif (state = 3) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch7_in;
				mult_in_vol_l <= signed("0" & vol_ch7_l);
				mult_in_vol_r <= signed("0" & vol_ch7_r);

				mult2_in_sample <= ch8_in;
				mult2_in_vol_l <= signed("0" & vol_ch8_l);
				mult2_in_vol_r <= signed("0" & vol_ch8_r);
				
				state <= state + 1;
			elsif (state = 4) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch9_in;
				mult_in_vol_l <= signed("0" & vol_ch9_l);
				mult_in_vol_r <= signed("0" & vol_ch9_r);

				mult2_in_sample <= ch10_in;
				mult2_in_vol_l <= signed("0" & vol_ch10_l);
				mult2_in_vol_r <= signed("0" & vol_ch10_r);
				
				state <= state + 1;
			elsif (state = 5) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch11_in;
				mult_in_vol_l <= signed("0" & vol_ch11_l);
				mult_in_vol_r <= signed("0" & vol_ch11_r);

				mult2_in_sample <= ch12_in;
				mult2_in_vol_l <= signed("0" & vol_ch12_l);
				mult2_in_vol_r <= signed("0" & vol_ch12_r);
				
				state <= state + 1;
			elsif (state = 6) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch13_in;
				mult_in_vol_l <= signed("0" & vol_ch13_l);
				mult_in_vol_r <= signed("0" & vol_ch13_r);

				mult2_in_sample <= ch14_in;
				mult2_in_vol_l <= signed("0" & vol_ch14_l);
				mult2_in_vol_r <= signed("0" & vol_ch14_r);
				
				state <= state + 1;
			elsif (state = 7) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch15_in;
				mult_in_vol_l <= signed("0" & vol_ch15_l);
				mult_in_vol_r <= signed("0" & vol_ch15_r);

				mult2_in_sample <= ch16_in;
				mult2_in_vol_l <= signed("0" & vol_ch16_l);
				mult2_in_vol_r <= signed("0" & vol_ch16_r);
				
				state <= state + 1;
			elsif (state = 8) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch17_in;
				mult_in_vol_l <= signed("0" & vol_ch17_l);
				mult_in_vol_r <= signed("0" & vol_ch17_r);

				mult2_in_sample <= ch18_in;
				mult2_in_vol_l <= signed("0" & vol_ch18_l);
				mult2_in_vol_r <= signed("0" & vol_ch18_r);
				
				state <= state + 1;
			elsif (state = 9) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch19_in;
				mult_in_vol_l <= signed("0" & vol_ch19_l);
				mult_in_vol_r <= signed("0" & vol_ch19_r);

				mult2_in_sample <= ch20_in;
				mult2_in_vol_l <= signed("0" & vol_ch20_l);
				mult2_in_vol_r <= signed("0" & vol_ch20_r);
				
				state <= state + 1;
			elsif (state = 10) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch21_in;
				mult_in_vol_l <= signed("0" & vol_ch21_l);
				mult_in_vol_r <= signed("0" & vol_ch21_r);

				mult2_in_sample <= ch22_in;
				mult2_in_vol_l <= signed("0" & vol_ch22_l);
				mult2_in_vol_r <= signed("0" & vol_ch22_r);
				
				state <= state + 1;

			elsif (state = 11) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch23_in;
				mult_in_vol_l <= signed("0" & vol_ch23_l);
				mult_in_vol_r <= signed("0" & vol_ch23_r);

				mult2_in_sample <= ch24_in;
				mult2_in_vol_l <= signed("0" & vol_ch24_l);
				mult2_in_vol_r <= signed("0" & vol_ch24_r);
				
				state <= state + 1;
	
			elsif (state = 12) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch25_in;
				mult_in_vol_l <= signed("0" & vol_ch25_l);
				mult_in_vol_r <= signed("0" & vol_ch25_r);

				mult2_in_sample <= ch26_in;
				mult2_in_vol_l <= signed("0" & vol_ch26_l);
				mult2_in_vol_r <= signed("0" & vol_ch26_r);
				
				state <= state + 1;

			elsif (state = 13) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch27_in;
				mult_in_vol_l <= signed("0" & vol_ch27_l);
				mult_in_vol_r <= signed("0" & vol_ch27_r);

				mult2_in_sample <= ch28_in;
				mult2_in_vol_l <= signed("0" & vol_ch28_l);
				mult2_in_vol_r <= signed("0" & vol_ch28_r);
				
				state <= state + 1;

			elsif (state = 14) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch29_in;
				mult_in_vol_l <= signed("0" & vol_ch29_l);
				mult_in_vol_r <= signed("0" & vol_ch29_r);

				mult2_in_sample <= ch30_in;
				mult2_in_vol_l <= signed("0" & vol_ch30_l);
				mult2_in_vol_r <= signed("0" & vol_ch30_r);
				
				state <= state + 1;

			elsif (state = 15) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch31_in;
				mult_in_vol_l <= signed("0" & vol_ch31_l);
				mult_in_vol_r <= signed("0" & vol_ch31_r);

				mult2_in_sample <= ch32_in;
				mult2_in_vol_l <= signed("0" & vol_ch32_l);
				mult2_in_vol_r <= signed("0" & vol_ch32_r);
				
				state <= state + 1;

			elsif (state = 16) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch33_in;
				mult_in_vol_l <= signed("0" & vol_ch33_l);
				mult_in_vol_r <= signed("0" & vol_ch33_r);

				mult2_in_sample <= ch34_in;
				mult2_in_vol_l <= signed("0" & vol_ch34_l);
				mult2_in_vol_r <= signed("0" & vol_ch34_r);
				
				state <= state + 1;

			elsif (state = 17) then
				main_l <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);

				mult_in_sample <= ch35_in;
				mult_in_vol_l <= signed("0" & vol_ch35_l);
				mult_in_vol_r <= signed("0" & vol_ch35_r);

				mult2_in_sample <= ch36_in;
				mult2_in_vol_l <= signed("0" & vol_ch36_l);
				mult2_in_vol_r <= signed("0" & vol_ch36_r);
				
				state <= state + 1;

			elsif (state = 18) then
				main_l_out <= main_l + resize(shift_right(mult_out_l, 7), main_l'length) + resize(shift_right(mult2_out_l, 7), main_l'length);
				main_r_out <= main_r + resize(shift_right(mult_out_r, 7), main_r'length) + resize(shift_right(mult2_out_r, 7), main_r'length);
				
				sync_out <= '1';
				state <= state + 1;
			
			-- TDM uses 12.288MHz clock-domain at 48kHz, but we use 100MHz here
			-- as we need 19 steps here, the sync-signal is low again
			-- but if the states are close to 8, use the following line:
			-- elsif (sync_in = '0' and state = 19) then
			elsif (state = 19) then
				sync_out <= '0';
				state <= 0;
			end if;
		end if;
	end process;
end Behavioral;