import { useEffect, useState } from "react";
import {
  collection,
  limit,
  onSnapshot,
  orderBy,
  query,
  type DocumentData,
  type QueryConstraint,
} from "firebase/firestore";
import { db } from "../firebase";

type HookResult<T> = {
  data: Array<T & { id: string }>;
  loading: boolean;
  error: Error | null;
};

export function useReadingsHistory<T extends DocumentData>(
  elevatorId: string,
  subcollectionName: string,
  options?: {
    limit?: number;
    orderField?: string;
    direction?: "asc" | "desc";
  },
): HookResult<T> {
  const [data, setData] = useState<Array<T & { id: string }>>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<Error | null>(null);

  useEffect(() => {
    if (!elevatorId || !subcollectionName) {
      setLoading(false);
      setData([]);
      setError(null);
      return;
    }

    setLoading(true);
    setError(null);

    const lim = options?.limit ?? 100;
    const orderField = options?.orderField ?? "timestamp";
    const direction = options?.direction ?? "desc";

    const colRef = collection(db, "elevator", elevatorId, subcollectionName);

    const constraints: QueryConstraint[] = [
      orderBy(orderField, direction),
      limit(lim),
    ];

    const q = query(colRef, ...constraints);

    const unsub = onSnapshot(
      q,
      (snap) => {
        setData(snap.docs.map((d) => ({ id: d.id, ...(d.data() as T) })));
        setLoading(false);
      },
      (err) => {
        setError(err instanceof Error ? err : new Error(String(err)));
        setLoading(false);
      },
    );

    return unsub;
  }, [
    elevatorId,
    subcollectionName,
    options?.limit,
    options?.orderField,
    options?.direction,
  ]);

  return { data, loading, error };
}
