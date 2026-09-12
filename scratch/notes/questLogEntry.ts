interface QuestLogEntry {
  questId: string;
  questLabel: string;
  questDescription: string;
  questCompletedDescription: string;
  stepOrder: string[];
  steps: {
    [stepId: string]: QuestLogStep | QuestLogMultiStep;
  };
}

interface QuestLogStep {
  stepLabel: string;
  stepDescription: string;
}

interface QuestLogMultiStep extends QuestLogStep {
  subStepOrder: string[];
  subSteps: {
    [stepId: string]: QuestLogStep;
  };
}

const CooksAssistant: QuestLogEntry = {
  questId: 'cooks-assistant',
  questLabel: 'Cooks Assistant',
  questDescription: 'Help the chef on floor 1 by finding the ingredients for a cake.',
  questCompletedDescription: 'You have helped the chef on floor 1 by finding the ingredients for a cake.',
  stepOrder: ['first', 'ingredients', 'finish'],
  steps: {
    first: {
      stepLabel: 'Floor 1 Chef.',
      stepDescription: 'The chef on floor 1 needs ingredients for a cake.',
    },
    ingredients: {
      stepLabel: 'Find ingredients.',
      stepDescription: 'The chef needs 3 ingredients that can be found somewhere.',
      subStepOrder: ['subStep1', 'subStep2', 'subStep3'],
      subSteps: {
        flour: {
          stepLabel: '',
          stepDescription: 'Find flour.',
        },
        sugar: {
          stepLabel: '',
          stepDescription: 'Find sugar.',
        },
        eggs: {
          stepLabel: '',
          stepDescription: 'Find eggs.',
        },
      },
    },
    finish: {
      stepLabel: 'Give ingredients to chef.',
      stepDescription: 'Return to the chef with all the ingredients.',
    },
  },
};
