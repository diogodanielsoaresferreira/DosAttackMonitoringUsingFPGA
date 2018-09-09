library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.all;

entity SegDisplay is
    Port (
        enDigit     :   in std_logic_vector (7 downto 0);
        enDP        :   in std_logic_vector (7 downto 0);
        value0      :   in std_logic_vector (3 downto 0);
        value1      :   in std_logic_vector (3 downto 0);
        value2      :   in std_logic_vector (3 downto 0);
        value3      :   in std_logic_vector (3 downto 0);
        value4      :   in std_logic_vector (3 downto 0);
        value5      :   in std_logic_vector (3 downto 0);
        value6      :   in std_logic_vector (3 downto 0);
        value7      :   in std_logic_vector (3 downto 0);
        clk         :   in std_logic;
        clkEnable   :   in std_logic;
        an          :   out std_logic_vector (7 downto 0);
        dp          :   out std_logic;
        seg         :   out std_logic_vector (6 downto 0)
    );
end SegDisplay;

architecture Behavioral of SegDisplay is
    signal s_counter    :   unsigned (2 downto 0) := "000";
    signal s_digitEn    :   std_logic;
    signal s_binSeg     :   std_logic_vector(3 downto 0);
begin
   
    -- 3-bit Counter
    process(clk, clkEnable)
        begin
        if (rising_edge(clk) and clkEnable='1') then
            s_counter <= s_counter + 1;
        end if;
    end process;
    

    -- Mux 8:1
    process(enDigit, s_counter)
    begin
        if(s_counter="111") then
            s_digitEn <= enDigit(7);
        elsif(s_counter="110") then
            s_digitEn <= enDigit(6);
        elsif(s_counter="101") then
            s_digitEn <= enDigit(5);
        elsif(s_counter="100") then
            s_digitEn <= enDigit(4);
        elsif(s_counter="011") then
            s_digitEn <= enDigit(3);
        elsif(s_counter="010") then
            s_digitEn <= enDigit(2);
        elsif(s_counter="001") then
            s_digitEn <= enDigit(1);
        else
            s_digitEn <= enDigit(0);
        end if;
    end process;
    
    -- Decoder 3:8
    process(s_counter)
    begin
        if(s_counter="111") then
            an <= "01111111";
        elsif(s_counter="110") then
            an <= "10111111";
        elsif(s_counter="101") then
            an <= "11011111";
        elsif(s_counter="100") then
            an <= "11101111";
        elsif(s_counter="011") then
            an <= "11110111";
        elsif(s_counter="010") then
            an <= "11111011";
        elsif(s_counter="001") then
            an <= "11111101";
        else
            an <= "11111110";
        end if;
    end process;

    -- Mux 8:1
    process(enDP, s_counter)
    begin
        if(s_counter="111") then
            dp <= not enDP(7);
        elsif(s_counter="110") then
            dp <= not enDP(6);
        elsif(s_counter="101") then
            dp <= not enDP(5);
        elsif(s_counter="100") then
            dp <= not enDP(4);
        elsif(s_counter="011") then
            dp <= not enDP(3);
        elsif(s_counter="010") then
            dp <= not enDP(2);
        elsif(s_counter="001") then
            dp <= not enDP(1);
        else
            dp <= not enDP(0);
        end if;
    end process;

    -- Mux 8:1 (4-bits)
    process(s_counter, value0, value1, value2, value3, value4, value5, value6, value7)
    begin
        if(s_counter="111") then
            s_binSeg <= value7;
        elsif(s_counter="110") then
            s_binSeg <= value6;
        elsif(s_counter="101") then
            s_binSeg <= value5;
        elsif(s_counter="100") then
            s_binSeg <= value4;
        elsif(s_counter="011") then
            s_binSeg <= value3;
        elsif(s_counter="010") then
            s_binSeg <= value2;
        elsif(s_counter="001") then
            s_binSeg <= value1;
        else
            s_binSeg <= value0;
        end if;
    end process;
    
    -- Bin7SegDec
    process(s_binSeg, s_digitEn)
    begin
        if(s_digitEn='1') then
            if(s_binSeg="0000") then
                seg <= "1000000";
            elsif(s_binSeg="0001") then
                seg <= "1111001";
            elsif(s_binSeg="0010") then
                seg <= "0100100";
            elsif(s_binSeg="0011") then
                seg <= "0110000";
            elsif(s_binSeg="0100") then
                seg <= "0011001";
            elsif(s_binSeg="0101") then
                seg <= "0010010";
            elsif(s_binSeg="0110") then
                seg <= "0000010";
            elsif(s_binSeg="0111") then
                seg <= "1111000";
            elsif(s_binSeg="1000") then
                seg <= "0000000";
            elsif(s_binSeg="1001") then
                seg <= "0010000";
            elsif(s_binSeg="1010") then
                seg <= "0001000";
            elsif(s_binSeg="1011") then
                seg <= "0000011";
            elsif(s_binSeg="1100") then
                seg <= "1000110";
            elsif(s_binSeg="1101") then
                seg <= "0100001";
            elsif(s_binSeg="1110") then
                seg <= "0000110";
            else
                seg <= "0001110";
            end if;
        else
            seg <= "1111111";
        end if;
    end process;

end Behavioral;
