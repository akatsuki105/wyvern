# Benchmark

## Tools

- wyvern: v1.1.2
- pb16: https://github.com/pinobatch/libbet/blob/8b894e4c02d43ddf2180176ee670e977b0170ccd/tools/pb16.py
- lz4gb: https://github.com/gitendo/lz4gb/tree/da333add4c956260e6db7c921d6903fe067acc3d
- pkmncompress: https://github.com/pret/pokered/tree/fc3bbc1358f95038b167d3f8bc9006ab5ab655c1/tools
- lzcomp: https://github.com/pret/pokecrystal/tree/7d3ea88611efbc2f9b903063e2816b1cae1bad35/tools

## Results

  Method | Size | Cycles | 
---- | ---- | ----
 wyvern & cenotaph.atr | 360 -> 199(55%) | TODO
 wyvern & cenotaph.chr | 4000 -> 3417(85%) | TODO
 wyvern & cenotaph.map | 360 -> 338(93%) | TODO
 wyvern & pokered/abra.2bpp | 400 -> 331(82%) | TODO
 wyvern & pokered/red.2bpp | 784 -> 340(43%) | TODO
 wyvern & pokecrystal/chris.2bpp | 784 -> 378(48%) | TODO 
 wyvern & pokecrystal/diploma.2bpp | 1792 -> 1202(67%) | TODO 
 wyvern & pokecrystal/ditto.2bpp | 1024 -> 437(42%) | TODO 
 wyvern & pokecrystal/suicune_jump.2bpp | 2048 -> 1295(63%) | TODO 
 pb16 & cenotaph.atr | 360 -> 226(62%) | TODO
 pb16 & cenotaph.chr | 4000 -> 3329(83%) | TODO
 pb16 & cenotaph.map | 360 -> 367(101%) | TODO
 pb16 & pokered/abra.2bpp | 400 -> 330(82%) | TODO
 pb16 & pokered/red.2bpp | 784 -> 393(50%) | TODO
 pb16 & pokecrystal/chris.2bpp | 784 -> 425(54%) | TODO 
 pb16 & pokecrystal/diploma.2bpp | 1792 -> 1037(57%) | TODO 
 pb16 & pokecrystal/ditto.2bpp | 1024 -> 477(46%) | TODO 
 pb16 & pokecrystal/suicune_jump.2bpp | 2048 -> 1330(64%) | TODO 
 lz4gb & cenotaph.atr | 360 -> 221(61%) | 26240
 lz4gb & cenotaph.chr | 4000 -> 3370(84%) | 223964
 lz4gb & cenotaph.map | 360 -> 342(95%) | 16676
 lz4gb & pokered/abra.2bpp | 400 -> 337(84%) | TODO
 lz4gb & pokered/red.2bpp | 784 -> 362(46%) | TODO
 lz4gb & pokecrystal/chris.2bpp | 784 -> 401(51%) | TODO 
 lz4gb & pokecrystal/diploma.2bpp | 1792 -> 1199(66%) | TODO 
 lz4gb & pokecrystal/ditto.2bpp | 1024 -> 422(41%) | TODO 
 lz4gb & pokecrystal/suicune_jump.2bpp | 2048 -> 1295(63%) | TODO 
 pkmncompress & pokered/abra.2bpp | 400 -> 259(64%) | TODO
 pkmncompress & pokered/red.2bpp  | 784 -> 266(33%) | TODO
 lzcomp & pokered/abra.2bpp | 400 -> 310(77%) | TODO 
 lzcomp & pokecrystal/chris.2bpp | 784 -> 365(46%) | TODO 
 lzcomp & pokecrystal/diploma.2bpp | 1792 -> 1094(61%) | TODO 
 lzcomp & pokecrystal/suicune_jump.2bpp | 2048 -> 1216(59%) | TODO 
