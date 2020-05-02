def tripleStep(n):
	if n < 0: return 0
	if n == 0: return 1

	dp = [0] * (n + 1)
	dp[0] = 1

	for i in range(1, n + 1):
		totalCombos = 0
		if i > 0: totalCombos += dp[i - 1]
		if i > 1: totalCombos += dp[i - 2]
		if i > 2: totalCombos += dp[i - 3]
		dp[i] = totalCombos

	return dp[n]

print(tripleStep(4))
