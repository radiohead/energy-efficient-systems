#ifndef DCT_LOOKUP_TABLE
#define DCT_LOOKUP_TABLE

float dct_lookup_table[64] __attribute__((aligned(16))) = {
	1.00000000000000000000,
	1.00000000000000000000,
	1.00000000000000000000,
	1.00000000000000000000,
	1.00000000000000000000,
	1.00000000000000000000,
	1.00000000000000000000,
	1.00000000000000000000,
	.98078528040323044912,
	.83146961230254523708,
	.55557023301960222475,
	.19509032201612826786,
	-.19509032201612826783,
	-.55557023301960222472,
	-.83146961230254523706,
	-.98078528040323044912,
	.92387953251128675613,
	.38268343236508977174,
	-.38268343236508977171,
	-.92387953251128675611,
	-.92387953251128675613,
	-.38268343236508977176,
	.38268343236508977168,
	.92387953251128675611,
	.83146961230254523708,
	-.19509032201612826783,
	-.98078528040323044912,
	-.55557023301960222476,
	.55557023301960222470,
	.98078528040323044913,
	.19509032201612826790,
	-.83146961230254523704,
	.70710678118654752440,
	-.70710678118654752438,
	-.70710678118654752442,
	.70710678118654752437,
	.70710678118654752443,
	-.70710678118654752435,
	-.70710678118654752445,
	.70710678118654752434,
	.55557023301960222475,
	-.98078528040323044912,
	.19509032201612826781,
	.83146961230254523711,
	-.83146961230254523704,
	-.19509032201612826793,
	.98078528040323044914,
	-.55557023301960222464,
	.38268343236508977174,
	-.92387953251128675613,
	.92387953251128675611,
	-.38268343236508977166,
	-.38268343236508977180,
	.92387953251128675616,
	-.92387953251128675608,
	.38268343236508977160,
	.19509032201612826786,
	-.55557023301960222476,
	.83146961230254523711,
	-.98078528040323044914,
	.98078528040323044910,
	-.83146961230254523701,
	.55557023301960222462,
	-.19509032201612826770
};
#endif
