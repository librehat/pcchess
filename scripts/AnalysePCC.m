function [simpr, speedup, winrate, avgscore] = AnalysePCC(outputfile)
% This function will calcuate the average simulations per round,
% speed-up, winning rate, and average score, using given output file.

data = load(outputfile);
rounds = size(data)(1);
simpr = 0;
blacksims = 0;
wins = 0;
avgscore = 0;
for i = 1 : rounds
    simpr += data(i, 5) / data(i, 2);
    blacksims += data(i, 6) / data(i, 2);
    if (data(i, 3) == 1)
        wins += 1;
    end
    avgscore += data(i, 3);
end
simpr /= rounds;
blacksims /= rounds;
speedup = simpr / blacksims;
winrate = wins / rounds;
avgscore /= rounds;
