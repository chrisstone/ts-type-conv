// Types based on TypeScript 5.9

// Interface Types, Union and Intersection Types
interface A {
	a: number;
};
interface B {
	b: string;
};
export type Union = A | B;
export type Intersection = A & B;

// Literal Types
export type Status = "success" | "error" | "pending";
export type Binary = 0 | 1;
export interface Result {
	status: Status;
	code: Binary;
};

// Template Literal Types
export type EventName = "click" | "hover" | "notReal";
export type ShoutEvents = Uppercase<EventName>;
export type QuietEvents = Lowercase<EventName>;
export type HandlerName = Capitalize<EventName>;
export type UncomfortableHandler = Uncapitalize<EventName>;

// Literal Inerpolation Unsupported. Do not emit.

// Mapped Types
type Flags = {
	optionA: boolean;
	optionB: boolean;
};

export type FeatureFlags = {
	[K in keyof Flags]?: boolean;
};

// typeof Unsupported. Do not emit.

// Array Types
export type NumberArray = number[];
export type StringArray = string[];
export type MixedArray = (number | string)[];
export type Users = Array<User>;
export type UsersReadonly = ReadonlyArray<User>;

// Tuples
export type Point3D = [number, number, number];
export type DynamicList = [string, ...number[]];
export type NamedTuple = [name: string, age: number];

// Unknowns
export interface SafeData {
	value: unknown;
	error: never;
};

// Enums
export enum Color {
	Red,
	Green,
	Blue
};
export enum Direction {
	Up = "UP",
	Down = "DOWN",
	Left = "LEFT",
	Right = "RIGHT"
};

// Utility Types
export interface User {
	id: number;
	name: string | undefined;
	age?: number;
};
// Awaited Unsupported. Do not emit.
export type PartialUser = Partial<User>;
export type ReadonlyUser = Readonly<User>;
export type RecordUser = Record<Color, User>;
export type PickUser = Pick<User, "id" | "name">;
export type OmitUser = Omit<User, "age">;

export type ExcludeColor = Exclude<Color, "Red">;
type PixelColor = {
	Red: number;
	Green: number;
	Blue: number;
};
export type ExtractColor = Extract<PixelColor, Color>;

export type NonNullableUser = NonNullable<User>;

// Function Utility Types are not supported. Do not emit:
// Parameters, ConstructorParameters, ReturnType, InstanceType, NoInfer, ThisParameterType, OmitThisParameter, ThisType
