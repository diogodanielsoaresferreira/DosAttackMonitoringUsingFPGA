library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity BloomFilterStream_v1_0_S00_AXIS is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- AXI4Stream sink: Data Width
		C_S_AXIS_TDATA_WIDTH	: integer	:= 32
	);
	port (
		-- Users to add ports here
        validData   : out std_logic;
        outData : out std_logic_vector(C_S_AXIS_TDATA_WIDTH-1 downto 0);
        readEnable  : in  std_logic;
        write       : in std_logic;
        reset       : in std_logic;
		-- User ports ends
		-- Do not modify the ports beyond this line

		-- AXI4Stream sink: Clock
		S_AXIS_ACLK	: in std_logic;
		-- AXI4Stream sink: Reset
		S_AXIS_ARESETN	: in std_logic;
		-- Ready to accept data in
		S_AXIS_TREADY	: out std_logic;
		-- Data in
		S_AXIS_TDATA	: in std_logic_vector(C_S_AXIS_TDATA_WIDTH-1 downto 0);
		-- Byte qualifier
		S_AXIS_TSTRB	: in std_logic_vector((C_S_AXIS_TDATA_WIDTH/8)-1 downto 0);
		-- Indicates boundary of last packet
		S_AXIS_TLAST	: in std_logic;
		-- Data is in valid
		S_AXIS_TVALID	: in std_logic
	);
end BloomFilterStream_v1_0_S00_AXIS;

architecture Behavioral of BloomFilterStream_v1_0_S00_AXIS is
    signal s_ready    : std_logic;
    signal s_validOut : std_logic;
    signal s_dataOut  : std_logic_vector(31 downto 0);
    
    signal s_m, s_m2, s_m3, s_m4    :   std_logic_vector (64-1 downto 0);
    signal s_b          :   std_logic_vector(3 downto 0);
    signal s_loadKey    :   std_logic;
    signal s_init_ready :   std_logic;
    signal s_hash_ready :   std_logic;
    signal s_init_r, s_init_r2, s_init_r3, s_init_r4    :   std_logic;
    signal s_hash_r, s_hash_r2, s_hash_r3, s_hash_r4    :   std_logic;
    signal s_init       :   std_logic;
    signal s_hash, s_hash2, s_hash3, s_hash4 :   std_logic_vector(64-1 downto 0);
    signal isMember, isMember2, isMember3, isMember4    :   std_logic;
    signal s_reset      :   std_logic;
    signal s_inputData  :   std_logic_vector(31 downto 0);
    signal s_writeBloom :   std_logic;
    signal s_dataValid  :   std_logic;
    
    type TState is (RESET_FSM, LOAD_FIRST_KEY, LOAD_SECOND_KEY, WAIT_HASH_DONE, CHECK_HASH_DONE,
        INIT_HASHING, FINALIZING_PROCESSING, WAIT_HASH_READY);
    signal CS, NS   :   TState;
    
    
    type BITARRAY is array(256 downto 0) of std_logic;
    signal bloom_filter, bloom_filter2, bloom_filter3, bloom_filter4 :   BITARRAY := (others => '0');
begin

    hash: entity work.siphash(behavioral)
            port map(
                m => s_m,
                b => s_b,
                rst_n => S_AXIS_ARESETN,
                clk => S_AXIS_ACLK,
                init => s_init,
                load_k => s_loadKey,
                init_ready => s_init_r,
                hash_ready => s_hash_r,
                hash => s_hash
            );

    hash2: entity work.siphash(behavioral)
            port map(
                m => s_m2,
                b => s_b,
                rst_n => S_AXIS_ARESETN,
                clk => S_AXIS_ACLK,
                init => s_init,
                load_k => s_loadKey,
                init_ready => s_init_r2,
                hash_ready => s_hash_r2,
                hash => s_hash2
            );

    hash3: entity work.siphash(behavioral)
            port map(
                m => s_m3,
                b => s_b,
                rst_n => S_AXIS_ARESETN,
                clk => S_AXIS_ACLK,
                init => s_init,
                load_k => s_loadKey,
                init_ready => s_init_r3,
                hash_ready => s_hash_r3,
                hash => s_hash3
            );

    hash4: entity work.siphash(behavioral)
            port map(
                m => s_m4,
                b => s_b,
                rst_n => S_AXIS_ARESETN,
                clk => S_AXIS_ACLK,
                init => s_init,
                load_k => s_loadKey,
                init_ready => s_init_r4,
                hash_ready => s_hash_r4,
                hash => s_hash4
            );

    s_reset <= not S_AXIS_ARESETN;

    s_init_ready <= s_init_r and s_init_r2 and s_init_r3 and s_init_r4;
    s_hash_ready <= s_hash_r and s_hash_r2 and s_hash_r3 and s_hash_r4;
    
    process(S_AXIS_ACLK)
    begin
        if(rising_edge(S_AXIS_ACLK)) then
            if(S_AXIS_ARESETN='0' or reset='1') then
                CS <= RESET_FSM;
            else
                CS <= NS;
            end if;
        end if;
    end process;
    
    process(S_AXIS_ACLK)
    begin
        if(rising_edge(S_AXIS_ACLK)) then
        
            if (CS=RESET_FSM) then
                bloom_filter <= (others => '0');
                bloom_filter2 <= (others => '0');
                bloom_filter3 <= (others => '0');
                bloom_filter4 <= (others => '0');
                s_inputData <= (others => '0');
            end if;
        
            if(S_AXIS_TVALID='1' and CS=WAIT_HASH_READY) then
                s_inputData <= S_AXIS_TDATA;
                s_dataValid <= '1';
            else
                s_dataValid <= '0';
            end if;

            if(s_writeBloom='1') then
                bloom_filter(to_integer(unsigned(s_hash(7 downto 0)))) <= '1';
                bloom_filter2(to_integer(unsigned(s_hash2(7 downto 0)))) <= '1';
                bloom_filter3(to_integer(unsigned(s_hash3(7 downto 0)))) <= '1';
                bloom_filter4(to_integer(unsigned(s_hash4(7 downto 0)))) <= '1';
            end if;
            
            
        end if;
    end process;
    
    process(CS, s_init_ready, s_hash_ready, write, readEnable, s_hash, s_hash2, s_hash3, s_hash4,
    isMember, isMember2, isMember3, isMember4, bloom_filter, bloom_filter2, bloom_filter3, bloom_filter4,
    s_inputData, s_dataValid)
    begin
        s_init <= '0';
        s_loadKey <= '0';
        s_ready <= '0';
        s_validOut <= '0';
        s_writeBloom <= '0';
        s_m <= (others => '0');
        s_m2 <= (others => '0');
        s_m3 <= (others => '0');
        s_m4 <= (others => '0');
        s_b <= (others => '0');
        isMember <= '0';
        isMember2 <= '0';
        isMember3 <= '0';
        isMember4 <= '0';
        s_dataOut <= (others => '0');
        NS <= CS;
        
        if (CS=RESET_FSM) then
            NS <= LOAD_FIRST_KEY;
        
        elsif (CS=LOAD_FIRST_KEY) then
            s_loadKey <= '1';
            s_m <= x"3fef8244f54a7d10";
            s_m2 <= x"65f863b640454d0e";
            s_m3 <= x"a5a75a3417aeb41c";
            s_m4 <= x"3e17bf0876b70f9f";
            NS <= LOAD_SECOND_KEY;
        
        elsif (CS=LOAD_SECOND_KEY) then
            s_loadKey <= '1';
            s_m <= x"647cb2b86ae8ff2a";
            s_m2 <= x"2b2a5a0ef4c4caba";
            s_m3 <= x"c88edcdb4780693d";
            s_m4 <= x"8c7994d107eb1cf6";
            NS <= WAIT_HASH_READY;
            
        elsif (CS=WAIT_HASH_READY) then
            s_ready <= '1';
            if(s_init_ready='1' and s_dataValid='1') then
                NS <= INIT_HASHING;
            end if;
        
        elsif (CS=INIT_HASHING) then
            s_init <= '1';
            s_m <= x"00000000" & s_inputData;
            s_m2 <= x"00000000" & s_inputData;
            s_m3 <= x"00000000" & s_inputData;
            s_m4 <= x"00000000" & s_inputData;
            s_b <= "0011";
            NS <= WAIT_HASH_DONE;
            
        elsif (CS=WAIT_HASH_DONE) then
            NS <= CHECK_HASH_DONE;
        
        elsif (CS=CHECK_HASH_DONE) then
            if(s_hash_ready='1' and s_init_ready='1') then
                if(write='1') then
                    s_writeBloom <= '1';
                end if;
                NS <= FINALIZING_PROCESSING;
            else
                NS <= CS;
            end if;
            
        elsif(CS=FINALIZING_PROCESSING) then
            isMember <= bloom_filter(to_integer(unsigned(s_hash(7 downto 0))));
            isMember2 <= bloom_filter2(to_integer(unsigned(s_hash2(7 downto 0))));
            isMember3 <= bloom_filter3(to_integer(unsigned(s_hash3(7 downto 0))));
            isMember4 <= bloom_filter4(to_integer(unsigned(s_hash4(7 downto 0))));
            
            s_validOut <= '1';
            s_dataOut <= "0000000000000000000000000000000" & (isMember and isMember2 and isMember3 and isMember4);
            if(readEnable = '1') then
                NS <= WAIT_HASH_READY;
            end if;
        end if;
    end process;
    
    validData      <= s_validOut;
	outData        <= s_dataOut;
	S_AXIS_TREADY  <= s_ready;
end Behavioral;

