function rollDice(numSides: number) {
  return Math.floor(Math.random() * numSides) + 1;
}

function testDiceDistribution(
  // numSides: number,
  // numDice: number,
  // numRolls: number
) {
  const numRolls = 10000;
  let rangedLevel = 10;
  let numDice = 1;

  const resultMap = new Map<number, number>();
  for (let i = 0; i < numRolls; i++) {
    const results: number[] = [];
    const result = Math.floor(Math.sqrt(rollDice(rangedLevel * (rangedLevel - 1))));
    resultMap.set(result, (resultMap.get(result) || 0) + 1);
  }
  return resultMap;
}

function printDiceDistribution(
  resultMap: Map<number, number>,
  maxWidth: number = 50
) {
  if (resultMap.size === 0) {
    console.log('No data to display');
    return;
  }

  // Find the maximum count for scaling
  const maxCount = Math.max(...Array.from(resultMap.values()));

  // Sort entries by key (dice result) for better visualization
  const sortedEntries = Array.from(resultMap.entries()).sort(
    (a, b) => a[0] - b[0]
  );

  let sum = 0;
  let totalCount = 0;

  // Print each entry as a horizontal bar
  for (const [result, count] of sortedEntries) {
    // Calculate the bar width proportional to maxCount
    const barWidth = Math.round((count / maxCount) * maxWidth);
    const bar = '█'.repeat(barWidth);

    // Format: "Result: count |████████..."
    console.log(
      `${result.toString().padStart(3)}: ${count
        .toString()
        .padStart(5)} |${bar}`
    );

    // For average calculation
    sum += result * count;
    totalCount += count;
  }

  if (totalCount > 0) {
    const average = sum / totalCount;
    console.log(`Average: ${average.toFixed(4)}`);
  }
}

printDiceDistribution(testDiceDistribution(), 50);
// console.log('--------------------------------');
// printDiceDistribution(testDiceDistribution(6, 2, 10000), 50);
// console.log('--------------------------------');
// printDiceDistribution(testDiceDistribution(6, 3, 10000), 50);
// console.log('--------------------------------');
// printDiceDistribution(testDiceDistribution(6, 4, 10000), 50);