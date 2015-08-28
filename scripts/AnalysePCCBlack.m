function [games, speedup, speederr, winrate, winerr] = AnalysePCCBlack(outputfile)
% this function calculates information below:
% - how many games presented in the output file
% - the speedup of black player against the red player
% - the standard error of the speedup
% - the win rate of black player
% - the standard error of the win rate

data = load(outputfile);
games = size(data)(1);
speedups = zeros(games, 1);
wins = zeros(games, 1); % 1: win, 0: lose

for i = 1 : games
    redsims = data(i, 5) / data(i, 2);
    blacksims = data(i, 6) / data(i, 2);
    speedups(i) = blacksims / redsims;
    if (data(i, 4) == 1)
        wins(i) = 1;
    end
end

speedup = mean(speedups);
speederr = std(speedups) / sqrt(games);
winrate = mean(wins);
winerr = std(wins) / sqrt(games);
