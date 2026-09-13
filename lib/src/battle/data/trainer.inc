/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

static const char* TRAINER_NAMES[] = {
    "None", // 0
    "Brendan", // 1
    "Brendan", // 2
    "Brendan", // 3
    "May", // 4
    "May", // 5
    "May", // 6
    "Youngster", // 7
    "Bug Catcher", // 8
    "Bug Catcher", // 9
    "Lass", // 10
    "Aroma Lady", // 11
    "Twins", // 12
    "Fisherman", // 13
    "Fisherman", // 14
    "Fisherman", // 15
    "Youngster", // 16
    "Lass", // 17
    "Schoolkid", // 18
    "Schoolkid", // 19
    "Team Aqua Grunt", // 20
    "Team Aqua Grunt", // 21
    "Youngster", // 22
    "Youngster", // 23
    "Bug Catcher", // 24
    "Lass", // 25
    "Swimmer", // 26
    "Swimmer", // 27
    "Swimmer", // 28
    "Swimmer", // 29
    "Swimmer", // 30
    "Swimmer", // 31
    "Fisherman", // 32
    "Fisherman", // 33
    "Guitarist", // 34
    "Guitarist", // 35
    "Swimmer", // 36
    "Swimmer", // 37
    "Swimmer", // 38
    "Swimmer", // 39
    "Swimmer", // 40
    "Swimmer", // 41
    "Swimmer", // 42
    "Swimmer", // 43
    "Swimmer", // 44
    "Swimmer", // 45
    "Sailor", // 46
    "Sailor", // 47
    "Tuber", // 48
    "Triathlete", // 49
    "Triathlete", // 50
    "Triathlete", // 51
    "Triathlete", // 52
    "Collector", // 53
    "Fisherman", // 54
    "Twins", // 55
    "Black Belt", // 56
    "Black Belt", // 57
    "Black Belt", // 58
    "Battle Girl", // 59
    "Battle Girl", // 60
    "Youngster", // 61
    "Youngster", // 62
    "Ace Trainer", // 63
    "Ace Trainer", // 64
    "Ace Trainer", // 65
    "Ace Trainer", // 66
    "Ace Trainer", // 67
    "Ace Trainer", // 68
    "Ace Trainer", // 69
    "Lady", // 70
    "Rich Boy", // 71
    "Pokémon Breeder", // 72
    "Pokémon Breeder", // 73
    "Beauty", // 74
    "Beauty", // 75
    "Lass", // 76
    "Hiker", // 77
    "Hiker", // 78
    "Camper", // 79
    "Picnicker", // 80
    "Kindler", // 81
    "Kindler", // 82
    "Kindler", // 83
    "Kindler", // 84
    "Kindler", // 85
    "Youngster", // 86
    "Youngster", // 87
    "Pokémon Breeder", // 88
    "Pokémon Breeder", // 89
    "Hiker", // 90
    "Hiker", // 91
    "Camper", // 92
    "Picnicker", // 93
    "Poké Maniac", // 94
    "Expert", // 95
    "Black Belt", // 96
    "Black Belt", // 97
    "Battle Girl", // 98
    "Collector", // 99
    "Aroma Lady", // 100
    "Aroma Lady", // 101
    "Bird Keeper", // 102
    "Bug Maniac", // 103
    "Bug Maniac", // 104
    "Bug Maniac", // 105
    "Bug Catcher", // 106
    "Bug Catcher", // 107
    "Bug Catcher", // 108
    "Pokémon Ranger", // 109
    "Pokémon Ranger", // 110
    "Bird Keeper", // 111
    "Bird Keeper", // 112
    "Ninja Boy", // 113
    "Ninja Boy", // 114
    "Bird Keeper", // 115
    "Bird Keeper", // 116
    "Bird Keeper", // 117
    "Bird Keeper", // 118
    "Bird Keeper", // 119
    "Bird Keeper", // 120
    "Ninja Boy", // 121
    "Ninja Boy", // 122
    "Pokémon Ranger", // 123
    "Pokémon Ranger", // 124
    "Bug Maniac", // 125
    "Pokémon Breeder", // 126
    "Pokémon Breeder", // 127
    "Poké Maniac", // 128
    "Dragon Tamer", // 129
    "Swimmer", // 130
    "Swimmer", // 131
    "Swimmer", // 132
    "Swimmer", // 133
    "Swimmer", // 134
    "Swimmer", // 135
    "Swimmer", // 136
    "Swimmer", // 137
    "Swimmer", // 138
    "Swimmer", // 139
    "Swimmer", // 140
    "Swimmer", // 141
    "Swimmer", // 142
    "Swimmer", // 143
    "Swimmer", // 144
    "Swimmer", // 145
    "Swimmer", // 146
    "Swimmer", // 147
    "Swimmer", // 148
    "Swimmer", // 149
    "Swimmer", // 150
    "Swimmer", // 151
    "Swimmer", // 152
    "Swimmer", // 153
    "Swimmer", // 154
    "Swimmer", // 155
    "Swimmer", // 156
    "Psychic", // 157
    "Psychic", // 158
    "Psychic", // 159
    "Psychic", // 160
    "Psychic", // 161
    "Psychic", // 162
    "Triathlete", // 163
    "Triathlete", // 164
    "Triathlete", // 165
    "Triathlete", // 166
    "Triathlete", // 167
    "Triathlete", // 168
    "Triathlete", // 169
    "Triathlete", // 170
    "Triathlete", // 171
    "Triathlete", // 172
    "Interviewers", // 173
    "Interviewers", // 174
    "Interviewers", // 175
    "Interviewers", // 176
    "Interviewers", // 177
    "Archie", // 178
    "Team Aqua Grunt", // 179
    "Team Aqua Grunt", // 180
    "Team Aqua Grunt", // 181
    "Team Aqua Grunt", // 182
    "Team Aqua Grunt", // 183
    "Team Aqua Grunt", // 184
    "Team Aqua Grunt", // 185
    "Team Aqua Grunt", // 186
    "Team Aqua Grunt", // 187
    "Team Aqua Grunt", // 188
    "Psychic", // 189
    "Psychic", // 190
    "Poké Maniac", // 191
    "Poké Maniac", // 192
    "Swimmer", // 193
    "Sailor", // 194
    "Youngster", // 195
    "Youngster", // 196
    "Youngster", // 197
    "Youngster", // 198
    "Lass", // 199
    "Lass", // 200
    "Lass", // 201
    "Lass", // 202
    "Black Belt", // 203
    "Black Belt", // 204
    "Black Belt", // 205
    "Team Aqua Grunt", // 206
    "Team Aqua Grunt", // 207
    "Sailor", // 208
    "Tuber", // 209
    "Youngster", // 210
    "Lass", // 211
    "Lass", // 212
    "Battle Girl", // 213
    "Battle Girl", // 214
    "Schoolkid", // 215
    "Schoolkid", // 216
    "Schoolkid", // 217
    "Camper", // 218
    "Picnicker", // 219
    "Hiker", // 220
    "Black Belt", // 221
    "Pokémon Ranger", // 222
    "Pokémon Ranger", // 223
    "Bird Keeper", // 224
    "Hex Maniac", // 225
    "Psychic", // 226
    "Psychic", // 227
    "Ace Trainer", // 228
    "Ace Trainer", // 229
    "Ace Trainer", // 230
    "Archie", // 231
    "Team Aqua Grunt", // 232
    "Team Aqua Grunt", // 233
    "Team Aqua Grunt", // 234
    "Maxie", // 235
    "Maxie", // 236
    "Team Magma Grunt", // 237
    "Team Magma Grunt", // 238
    "Team Magma Grunt", // 239
    "Team Magma Grunt", // 240
    "Team Magma Grunt", // 241
    "Team Magma Grunt", // 242
    "Team Magma Grunt", // 243
    "Team Magma Grunt", // 244
    "Team Magma Grunt", // 245
    "Team Magma Grunt", // 246
    "Team Magma Grunt", // 247
    "Team Magma Grunt", // 248
    "Team Magma Grunt", // 249
    "Team Magma Grunt", // 250
    "Team Magma Grunt", // 251
    "Team Magma Grunt", // 252
    "Team Magma Grunt", // 253
    "Team Aqua Grunt", // 254
    "Team Magma Grunt", // 255
    "Team Aqua Grunt", // 256
    "Team Aqua Grunt", // 257
    "Team Aqua Grunt", // 258
    "Team Aqua Grunt", // 259
    "Team Aqua Grunt", // 260
    "Team Magma Grunt", // 261
    "Team Magma Grunt", // 262
    "Team Magma Grunt", // 263
    "Team Magma Grunt", // 264
    "Team Magma Grunt", // 265
    "Archie", // 266
    "Team Aqua Grunt", // 267
    "Team Aqua Grunt", // 268
    "Team Magma Grunt", // 269
    "Team Magma Grunt", // 270
    "Maxie", // 271
    "Fisherman", // 272
    "Hiker", // 273
    "Triathlete", // 274
    "Twins", // 275
    "Twins", // 276
    "Team Aqua Grunt", // 277
    "Team Magma Grunt", // 278
    "Hiker", // 279
    "Camper", // 280
    "Picnicker", // 281
    "Twins", // 282
    "Team Aqua Grunt", // 283
    "Team Aqua Grunt", // 284
    "Team Aqua Grunt", // 285
    "Team Magma Grunt", // 286
    "Team Magma Grunt", // 287
    "Team Magma Grunt", // 288
    "Brendan", // 289
    "Brendan", // 290
    "Brendan", // 291
    "Brendan", // 292
    "Brendan", // 293
    "Brendan", // 294
    "May", // 295
    "May", // 296
    "May", // 297
    "May", // 298
    "May", // 299
    "May", // 300
    "Youngster", // 301
    "Bug Catcher", // 302
    "Camper", // 303
    "Picnicker", // 304
    "Youngster", // 305
    "Bug Maniac", // 306
    "Hiker", // 307
    "Picnicker", // 308
    "Camper", // 309
    "Ace Trainer", // 310
    "Ace Trainer", // 311
    "Black Belt", // 312
    "Beauty", // 313
    "Ninja Boy", // 314
    "Collector", // 315
    "Fisherman", // 316
    "Fisherman", // 317
    "Bird Keeper", // 318
    "Aroma Lady", // 319
    "Hex Maniac", // 320
    "Psychic", // 321
    "Psychic", // 322
    "Ace Trainer", // 323
    "Parasol Lady", // 324
    "Parasol Lady", // 325
    "Hex Maniac", // 326
    "Gentleman", // 327
    "Team Aqua Grunt", // 328
    "Team Magma Grunt", // 329
    "Hex Maniac", // 330
    "Hex Maniac", // 331
    "Young Couple", // 332
    "Black Belt", // 333
    "Team Aqua Grunt", // 334
    "Team Aqua Grunt", // 335
    "Team Magma Grunt", // 336
    "Team Magma Grunt", // 337
    "Beauty", // 338
    "Beauty", // 339
    "Beauty", // 340
    "Lass", // 341
    "Lass", // 342
    "Ace Trainer", // 343
    "Ace Trainer", // 344
    "Ace Trainer", // 345
    "Ace Trainer", // 346
    "Ace Trainer", // 347
    "Ace Trainer", // 348
    "Ace Trainer", // 349
    "Ace Trainer", // 350
    "Ace Trainer", // 351
    "Ace Trainer", // 352
    "Team Aqua Grunt", // 353
    "Gentleman", // 354
    "Gentleman", // 355
    "Young Couple", // 356
    "Sailor", // 357
    "Sailor", // 358
    "Tuber", // 359
    "Fisherman", // 360
    "Tuber", // 361
    "Tuber", // 362
    "Sailor", // 363
    "Young Couple", // 364
    "Expert", // 365
    "Kindler", // 366
    "Guitarist", // 367
    "Twins", // 368
    "Fisherman", // 369
    "Fisherman", // 370
    "Fisherman", // 371
    "Fisherman", // 372
    "Tuber", // 373
    "Tuber", // 374
    "Tuber", // 375
    "Tuber", // 376
    "Triathlete", // 377
    "Triathlete", // 378
    "Triathlete", // 379
    "Triathlete", // 380
    "Triathlete", // 381
    "Triathlete", // 382
    "Triathlete", // 383
    "Triathlete", // 384
    "Collector", // 385
    "Collector", // 386
    "Collector", // 387
    "Collector", // 388
    "Ace Trainer", // 389
    "Ace Trainer", // 390
    "Ace Trainer", // 391
    "Ace Trainer", // 392
    "Ace Trainer", // 393
    "Ace Trainer", // 394
    "Ace Trainer", // 395
    "Ace Trainer", // 396
    "Hiker", // 397
    "Hiker", // 398
    "Hiker", // 399
    "Hiker", // 400
    "Poké Maniac", // 401
    "Poké Maniac", // 402
    "Poké Maniac", // 403
    "Poké Maniac", // 404
    "Kindler", // 405
    "Kindler", // 406
    "Kindler", // 407
    "Kindler", // 408
    "Battle Girl", // 409
    "Battle Girl", // 410
    "Battle Girl", // 411
    "Battle Girl", // 412
    "Schoolkid", // 413
    "Schoolkid", // 414
    "Schoolkid", // 415
    "Schoolkid", // 416
    "Schoolkid", // 417
    "Schoolkid", // 418
    "Schoolkid", // 419
    "Schoolkid", // 420
    "Aroma Lady", // 421
    "Aroma Lady", // 422
    "Aroma Lady", // 423
    "Aroma Lady", // 424
    "Guitarist", // 425
    "Guitarist", // 426
    "Guitarist", // 427
    "Guitarist", // 428
    "Pokémon Ranger", // 429
    "Pokémon Ranger", // 430
    "Pokémon Ranger", // 431
    "Pokémon Ranger", // 432
    "Pokémon Ranger", // 433
    "Pokémon Ranger", // 434
    "Pokémon Ranger", // 435
    "Pokémon Ranger", // 436
    "Bird Keeper", // 437
    "Bird Keeper", // 438
    "Bird Keeper", // 439
    "Bird Keeper", // 440
    "Bug Maniac", // 441
    "Bug Maniac", // 442
    "Bug Maniac", // 443
    "Gentleman", // 444
    "Psychic", // 445
    "Psychic", // 446
    "Psychic", // 447
    "Psychic", // 448
    "Psychic", // 449
    "Psychic", // 450
    "Psychic", // 451
    "Psychic", // 452
    "Sailor", // 453
    "Sailor", // 454
    "Sailor", // 455
    "Sailor", // 456
    "Expert", // 457
    "Expert", // 458
    "Expert", // 459
    "Expert", // 460
    "Hex Maniac", // 461
    "Hex Maniac", // 462
    "Hex Maniac", // 463
    "Hex Maniac", // 464
    "Young Couple", // 465
    "Young Couple", // 466
    "Young Couple", // 467
    "Young Couple", // 468
    "Camper", // 469
    "Camper", // 470
    "Camper", // 471
    "Camper", // 472
    "Picnicker", // 473
    "Picnicker", // 474
    "Picnicker", // 475
    "Picnicker", // 476
    "Bug Catcher", // 477
    "Bug Catcher", // 478
    "Bug Catcher", // 479
    "Bug Catcher", // 480
    "Dragon Tamer", // 481
    "Dragon Tamer", // 482
    "Dragon Tamer", // 483
    "Triathlete", // 484
    "Triathlete", // 485
    "Triathlete", // 486
    "Triathlete", // 487
    "Triathlete", // 488
    "Triathlete", // 489
    "Triathlete", // 490
    "Triathlete", // 491
    "Triathlete", // 492
    "Triathlete", // 493
    "Triathlete", // 494
    "Triathlete", // 495
    "Triathlete", // 496
    "Triathlete", // 497
    "Triathlete", // 498
    "Triathlete", // 499
    "Triathlete", // 500
    "Triathlete", // 501
    "Fisherman", // 502
    "Ace Trainer", // 503
    "Bird Keeper", // 504
    "Dragon Tamer", // 505
    "Bird Keeper", // 506
    "Swimmer", // 507
    "Swimmer", // 508
    "Swimmer", // 509
    "Swimmer", // 510
    "Swimmer", // 511
    "Swimmer", // 512
    "Swimmer", // 513
    "Swimmer", // 514
    "Beauty", // 515
    "Camper", // 516
    "Picnicker", // 517
    "Wally", // 518
    "Pokémon Breeder", // 519
    "Pokémon Breeder", // 520
    "Pokémon Breeder", // 521
    "Pokémon Breeder", // 522
    "Pokémon Breeder", // 523
    "Pokémon Breeder", // 524
    "Pokémon Breeder", // 525
    "Pokémon Breeder", // 526
    "Brendan", // 527
    "Brendan", // 528
    "Brendan", // 529
    "May", // 530
    "May", // 531
    "May", // 532
    "Fisherman", // 533
    "Fisherman", // 534
    "Fisherman", // 535
    "Ace Trainer", // 536
    "Ace Trainer", // 537
    "Black Belt", // 538
    "Fisherman", // 539
    "Bird Keeper", // 540
    "Swimmer", // 541
    "Swimmer", // 542
    "Twins", // 543
    "Sis and Bro", // 544
    "Sis and Bro", // 545
    "Sis and Bro", // 546
    "Sis and Bro", // 547
    "Sis and Bro", // 548
    "Sis and Bro", // 549
    "Sis and Bro", // 550
    "Fisherman", // 551
    "Leader Tate & Liza", // 552
    "Elite Four Sidney", // 553
    "Elite Four Phoebe", // 554
    "Elite Four Glacia", // 555
    "Elite Four Drake", // 556
    "Champion Steven", // 557
    "Poké Fan", // 558
    "Lady", // 559
    "Rich Boy", // 560
    "Leader Roxanne", // 561
    "Youngster", // 562
    "Leader Brawly", // 563
    "Tuber", // 564
    "Psychic", // 565
    "Psychic", // 566
    "Leader Wattson", // 567
    "Youngster", // 568
    "Leader Flannery", // 569
    "Leader Norman", // 570
    "Leader Winona", // 571
    "Leader Wallace", // 572
    "Camper", // 573
    "Picnicker", // 574
    "Camper", // 575
    "Picnicker", // 576
    "Ruin Maniac", // 577
    "Expert", // 578
    "Parasol Lady", // 579
    "Ace Trainer", // 580
    "Ace Trainer", // 581
    "Interviewers", // 582
    "Wally", // 583
    "Senior & Junior", // 584
    "Senior & Junior", // 585
    "Twins", // 586
    "Lady", // 587
    "Lady", // 588
    "Hiker", // 589
    "Hiker", // 590
    "Ninja Boy", // 591
    "Ruin Maniac", // 592
    "Beauty", // 593
    "Beauty", // 594
    "Ruin Maniac", // 595
    "Twins", // 596
    "Twins", // 597
    "Tuber", // 598
    "Tuber", // 599
    "Tuber", // 600
    "Tuber", // 601
    "Ruin Maniac", // 602
    "Ruin Maniac", // 603
    "Ruin Maniac", // 604
    "Ruin Maniac", // 605
    "Expert", // 606
    "Expert", // 607
    "Expert", // 608
    "Expert", // 609
    "Gentleman", // 610
    "Gentleman", // 611
    "Gentleman", // 612
    "Ace Trainer", // 613
    "Battle Girl", // 614
    "Battle Girl", // 615
    "Ninja Boy", // 616
    "Ninja Boy", // 617
    "Ninja Boy", // 618
    "Parasol Lady", // 619
    "Parasol Lady", // 620
    "Parasol Lady", // 621
    "Parasol Lady", // 622
    "Senior & Junior", // 623
    "Senior & Junior", // 624
    "Senior & Junior", // 625
    "Senior & Junior", // 626
    "Beauty", // 627
    "Beauty", // 628
    "Beauty", // 629
    "Beauty", // 630
    "Senior & Junior", // 631
    "Senior & Junior", // 632
    "Young Couple", // 633
    "Rich Boy", // 634
    "Rich Boy", // 635
    "Rich Boy", // 636
    "Rich Boy", // 637
    "Rich Boy", // 638
    "Rich Boy", // 639
    "Lady", // 640
    "Poké Fan", // 641
    "Poké Fan", // 642
    "Black Belt", // 643
    "Poké Fan", // 644
    "Poké Fan", // 645
    "Lady", // 646
    "Poké Fan", // 647
    "Lady", // 648
    "Rich Boy", // 649
    "Poké Fan", // 650
    "Poké Fan", // 651
    "Poké Fan", // 652
    "Poké Fan", // 653
    "Poké Fan", // 654
    "Lady", // 655
    "Lady", // 656
    "Lady", // 657
    "Rich Boy", // 658
    "Rich Boy", // 659
    "Rich Boy", // 660
    "Poké Fan", // 661
    "Poké Fan", // 662
    "Poké Fan", // 663
    "Poké Fan", // 664
    "Bug Maniac", // 665
    "Dragon Tamer", // 666
    "Schoolkid", // 667
    "Team Magma Grunt", // 668
    "Team Magma Grunt", // 669
    "Team Magma Grunt", // 670
    "Team Aqua Grunt", // 671
    "Team Aqua Grunt", // 672
    "Team Aqua Grunt", // 673
    "Brendan", // 674
    "Brendan", // 675
    "Brendan", // 676
    "May", // 677
    "May", // 678
    "May", // 679
    "Champion Steven", // 680
    "Inver", // 681
    "Rotation Trainer", // 682
    "Team Aqua Matt / Shelly", // 683
    "Team Aqua Matt / Shelly", // 684
    "Team Aqua Matt / Shelly", // 685
    "Team Aqua Matt / Shelly", // 686
    "Team Aqua Matt / Shelly", // 687
    "Team Aqua Shelly", // 688
    "Team Aqua Shelly", // 689
    "Team Aqua Shelly", // 690
    "Team Magma Tabitha / Court", // 691
    "Team Magma Tabitha / Court", // 692
    "Team Magma Tabitha / Court", // 693
    "Team Magma Courtney", // 694
    "Team Magma Courtney", // 695
    "Team Magma Courtney", // 696
    "Team Magma Courtney", // 697
    "Team Magma Courtney", // 698
    "Brendan", // 699
    "Brendan", // 700
    "Brendan", // 701
    "Collector", // 702
    "Collector", // 703
    "Collector", // 704
    "Collector", // 705
    "Collector", // 706
    "Collector", // 707
    "Collector", // 708
    "Poké Maniac", // 709
    "Bug Maniac", // 710
    "Aroma Lady", // 711
    "Fairy Tale Girl", // 712
    "Zinnia", // 713
    "Triathlete", // 714
    "Triathlete", // 715
    "Triathlete", // 716
    "Triathlete", // 717
    "Triathlete", // 718
    "Triathlete", // 719
    "Backpacker", // 720
    "Backpacker", // 721
    "Backpacker", // 722
    "Backpacker", // 723
    "Backpacker", // 724
    "Backpacker", // 725
    "Backpacker", // 726
    "Backpacker", // 727
    "Backpacker", // 728
    "Battle Girl", // 729
    "Bird Keeper", // 730
    "Brain & Brawn", // 731
    "Brain & Brawn", // 732
    "Brain & Brawn", // 733
    "Brain & Brawn", // 734
    "Brain & Brawn", // 735
    "Brain & Brawn", // 736
    "Scuba Diver", // 737
    "Scuba Diver", // 738
    "Scuba Diver", // 739
    "Scuba Diver", // 740
    "Scuba Diver", // 741
    "Scuba Diver", // 742
    "Scuba Diver", // 743
    "Scuba Diver", // 744
    "Scuba Diver", // 745
    "Scuba Diver", // 746
    "Scuba Diver", // 747
    "Scuba Diver", // 748
    "Scuba Diver", // 749
    "Scuba Diver", // 750
    "Scuba Diver", // 751
    "Scuba Diver", // 752
    "Scuba Diver", // 753
    "Scuba Diver", // 754
    "Scuba Diver", // 755
    "Scuba Diver", // 756
    "Scuba Diver", // 757
    "Scuba Diver", // 758
    "Scuba Diver", // 759
    "Scuba Diver", // 760
    "Scuba Diver", // 761
    "Scuba Diver", // 762
    "Dragon Tamer", // 763
    "Dragon Tamer", // 764
    "Expert", // 765
    "Fairy Tale Girl", // 766
    "Fairy Tale Girl", // 767
    "Fairy Tale Girl", // 768
    "Fairy Tale Girl", // 769
    "Fairy Tale Girl", // 770
    "Fairy Tale Girl", // 771
    "Fairy Tale Girl", // 772
    "Fairy Tale Girl", // 773
    "Fisherman", // 774
    "Fisherman", // 775
    "Fisherman", // 776
    "Fisherman", // 777
    "Fisherman", // 778
    "Fisherman", // 779
    "Mysterious Sister", // 780
    "Mysterious Sister", // 781
    "Mysterious Sister", // 782
    "Mysterious Sister", // 783
    "Mysterious Sister", // 784
    "Mysterious Sister", // 785
    "Ruin Maniac", // 786
    "Ruin Maniac", // 787
    "Hiker", // 788
    "Street Thug", // 789
    "Street Thug", // 790
    "Street Thug", // 791
    "Street Thug", // 792
    "Street Thug", // 793
    "Street Th Thug", // 794
    "Street Thug", // 795
    "Street Thug", // 796
    "Street Thug", // 797
    "Delinquent", // 798
    "Delinquent", // 799
    "Delinquent", // 800
    "Delinquent", // 801
    "Delinquent", // 802
    "Delinquent", // 803
    "Delinquent", // 804
    "Delinquent", // 805
    "Surfer", // 806
    "Surfer", // 807
    "Surfer", // 808
    "Surfer", // 809
    "Surfer", // 810
    "Surfer", // 811
    "Surfer", // 812
    "Surfer", // 813
    "Surfer", // 814
    "Surfer", // 815
    "Surfer", // 816
    "Surfer", // 817
    "Surfer", // 818
    "Surfer", // 819
    "Surfer", // 820
    "Surfer", // 821
    "Surfer", // 822
    "Ninja Boy", // 823
    "Ninja Boy", // 824
    "Swimmer", // 825
    "Swimmer", // 826
    "Tuber", // 827
    "Tuber", // 828
    "Veteran", // 829
    "Veteran", // 830
    "Veteran", // 831
    "Veteran", // 832
    "Veteran", // 833
    "Veteran", // 834
    "Expert", // 835
    "Youngster", // 836
    "Youngster", // 837
    "Schoolkid", // 838
    "Schoolkid", // 839
    "Fairy Tale Girl", // 840
    "Lass", // 841
    "Aroma Lady", // 842
    "Ace Trainer", // 843
    "Ace Trainer", // 844
    "Street Thug", // 845
    "Street Thug", // 846
    "Street Thug", // 847
    "Street Thug", // 848
    "Street Thug", // 849
    "Street Thug", // 850
    "Street Thug", // 851
    "Black Belt", // 852
    "Rich Boy", // 853
    "Lady", // 854
    "Rich Boy", // 855
    "Chaz", // 856
    "Chaz", // 857
    "Team Magma Grunt", // 858
    "Team Magma Grunt", // 859
    "Team Aqua Grunt", // 860
    "Team Aqua Grunt", // 861
    "Team Magma Grunt", // 862
    "Hiker", // 863
    "Schoolkid", // 864
    "Schoolkid", // 865
    "Black Belt", // 866
    "Poké Maniac", // 867
    "Poké Fan", // 868
    "Poké Fan", // 869
    "Pokémon Ranger", // 870
    "Pokémon Ranger", // 871
    "Swimmer", // 872
    "Ace Trainer", // 873
    "Ace Trainer", // 874
    "Street Thug", // 875
    "Delinquent", // 876
    "Expert", // 877
    "Expert", // 878
    "Rich Boy", // 879
    "Ninja Boy", // 880
    "Guitarist", // 881
    "Pokémon Breeder", // 882
    "Pokémon Breeder", // 883
    "Camper", // 884
    "Aroma Lady", // 885
    "Battle Girl", // 886
    "Fairy Tale Girl", // 887
    "Parasol Lady", // 888
    "Lady", // 889
    "Beauty", // 890
    "Lass", // 891
    "Team Magma Grunt", // 892
    "Team Magma Grunt", // 893
    "Team Magma Grunt", // 894
    "Team Aqua Grunt", // 895
    "Team Aqua Grunt", // 896
    "Team Aqua Grunt", // 897
    "Zinnia", // 898
    "Picnicker", // 899
    "Team Magma Grunt", // 900
    "Team Magma Grunt", // 901
    "Team Magma Grunt", // 902
    "Team Aqua Grunt", // 903
    "Team Aqua Grunt", // 904
    "Team Aqua Grunt", // 905
    "May", // 906
    "May", // 907
    "May", // 908
    "Elite Four Sidney", // 909
    "Elite Four Phoebe", // 910
    "Elite Four Glacia", // 911
    "Elite Four Drake", // 912
    "Champion Steven", // 913
    "Ninja Boy", // 914
    "Ninja Boy", // 915
    "Ninja Boy", // 916
    "Ninja Boy", // 917
    "Poké Maniac", // 918
    "Poké Maniac", // 919
    "Poké Maniac", // 920
    "Poké Maniac", // 921
    "Poké Maniac", // 922
    "Poké Maniac", // 923
    "Aroma Lady", // 924
    "Aroma Lady", // 925
    "Aroma Lady", // 926
    "Aroma Lady", // 927
    "Aroma Lady", // 928
    "Aroma Lady", // 929
    "Fairy Tale Girl", // 930
    "Fairy Tale Girl", // 931
    "Fairy Tale Girl", // 932
    "Fairy Tale Girl", // 933
    "Fairy Tale Girl", // 934
    "Fairy Tale Girl", // 935
    "Bug Maniac", // 936
    "Bug Maniac", // 937
    "Bug Maniac", // 938
    "Bug Maniac", // 939
    "Bug Maniac", // 940
    "Bug Maniac", // 941
    "Champion Steven", // 942
    "Leader Wallace", // 943
    "Wally", // 944
    "Wally", // 945
    "Wally", // 946
    "Wally", // 947
    "Team Aqua Grunt", // 948
    "Team Magma Grunt" // 949
};