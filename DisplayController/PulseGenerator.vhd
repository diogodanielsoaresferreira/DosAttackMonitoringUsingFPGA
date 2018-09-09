----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 01.03.2018 21:20:17
-- Design Name: 
-- Module Name: PulseGenerator - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity PulseGenerator is
    generic( N  :   natural);
    Port (
        clk     :   in std_logic;
        pulse   :   out std_logic
    );
end PulseGenerator;

architecture Behavioral of PulseGenerator is
    signal s_counter    :   natural := 0;
begin

    process(clk)
    begin
        if(rising_edge(clk)) then
            if (s_counter = N-1) then
                s_counter <= 0;
                pulse <= '1';
            else
                s_counter <= s_counter + 1;
                pulse <= '0';
            end if;
        end if;
    
    end process;

end Behavioral;
