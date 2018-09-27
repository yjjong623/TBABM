#!/bin/bash
csvjson "sampleParams.csv" > "sampleParams.json"

csvjson "Demographic/probability of pregnant.csv"       > "Demographic/probability of pregnant.json"
csvjson "Demographic/time in marriage.csv"              > "Demographic/time in marriage.json"
csvjson "Demographic/time to first birth.csv"           > "Demographic/time to first birth.json"
csvjson "Demographic/time to looking.csv"               > "Demographic/time to looking.json"
csvjson "Demographic/time to natural death.csv"         > "Demographic/time to natural death.json"
csvjson "Demographic/time to natural death newdata.csv" > "Demographic/time to natural death newdata.json"
csvjson "Demographic/time to natural death fixed.csv"   > "Demographic/time to natural death fixed.json"
csvjson "Demographic/time to subsequent births.csv"     > "Demographic/time to subsequent births.json"

csvjson "HIV/ART - TB.csv"                  > "HIV/ART - TB.json"
csvjson "HIV/ART - noTB.csv"                > "HIV/ART - noTB.json"
csvjson "HIV/VCT a_t_i.csv"                 > "HIV/VCT a_t_i.json"
csvjson "HIV/VCT sig_i.csv"                 > "HIV/VCT sig_i.json"
csvjson "HIV/infection risk - nospouse.csv" > "HIV/infection risk - nospouse.json"
csvjson "HIV/infection risk - spouse.csv"   > "HIV/infection risk - spouse.json"