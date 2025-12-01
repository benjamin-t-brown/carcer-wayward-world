import {
  GameEvent,
  GameEventChildChoice,
  GameEventChildExec,
  GameEventChildSwitch,
  GameEventChildType,
  SENode,
} from '../types/assets';

interface ValidationError {
  childId: string;
  message: string;
}

export class EventValidator {
  gameEvent: GameEvent;
  errors: ValidationError[] = [];

  constructor(gameEvent: GameEvent) {
    this.gameEvent = gameEvent;
  }

  private validateExecNode(seNode: GameEventChildExec) {
    if (!seNode.next) {
      this.errors.push({
        childId: seNode.id,
        message: 'Exec node must have a next node.',
      });
    }
  }

  private validateSwitchNode(seNode: GameEventChildSwitch) {
    if (!seNode.defaultNext) {
      this.errors.push({
        childId: seNode.id,
        message: 'Switch node must have a default next node.',
      });
    }

    for (const c of seNode.cases) {
      if (!c.next) {
        this.errors.push({
          childId: seNode.id,
          message: 'Switch node must have a next node for each case.',
        });
      }
    }
  }

  private validateChoiceNode(seNode: GameEventChildChoice) {
    if (!seNode.choices.length) {
      this.errors.push({
        childId: seNode.id,
        message: 'Choice node must have at least one choice.',
      });
    }

    for (const c of seNode.choices) {
      if (!c.next) {
        this.errors.push({
          childId: seNode.id,
          message: 'Choice node must have a next node for each choice.',
        });
      }
    }
  }

  validateSeNode(seNode: SENode) {
    switch (seNode.eventChildType) {
      case GameEventChildType.EXEC:
        this.validateExecNode(seNode as GameEventChildExec);
        break;
      case GameEventChildType.SWITCH:
        this.validateSwitchNode(seNode as GameEventChildSwitch);
        break;
      case GameEventChildType.CHOICE:
        this.validateChoiceNode(seNode as GameEventChildChoice);
        break;
      case GameEventChildType.END:
        break;
      default:
        throw new Error(`Unknown child type: ${seNode.eventChildType}`);
    }
  }

  validate() {
    for (const child of this.gameEvent.children) {
      this.validateSeNode(child);
    }
    return this.errors;
  }
}
