import { RootType } from "./import_out";

export module In {
	export type InType = {
		root: RootType;
	};
}

export namespace Out {
	export type OutType = {
		OtherRoot: RootType;
	};
}
