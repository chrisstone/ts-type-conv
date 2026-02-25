export namespace Service {
	export interface Request {
		id: number;
		query: string;
		flags: string[];
	}

	export interface Response {
		success: boolean;
		message?: string;
	}

	export enum ErrorCode {
		UNKNOWN = 0,
		NOT_FOUND = 1,
		TIMEOUT = 2
	}
}
