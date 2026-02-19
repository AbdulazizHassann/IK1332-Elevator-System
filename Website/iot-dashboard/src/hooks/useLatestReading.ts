import { useEffect, useState } from "react";
import {
  collection,
  limit,
  onSnapshot,
  orderBy,
  query,
  type DocumentData,
} from "firebase/firestore";
import { db } from "../firebase";

type Result<T> = {
  data: (T & { id: string }) | null;
  loading: boolean;
  error: Error | null;
};

export function useLatestReading<T extends DocumentData>(
  elevatorId: string,
  subcollectionName: string,
  orderField: string = "timestamp",
): Result<T> {
  const [data, setData] = useState<(T & { id: string }) | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<Error | null>(null);

  useEffect(() => {
    if (!elevatorId || !subcollectionName) {
      setLoading(false);
      return;
    }

    setLoading(true);
    setError(null);

    const colRef = collection(db, "elevator", elevatorId, subcollectionName);
    const q = query(colRef, orderBy(orderField, "desc"), limit(1));

    const unsub = onSnapshot(
      q,
      (snap) => {
        const d = snap.docs[0];
        setData(d ? { id: d.id, ...(d.data() as T) } : null);
        setLoading(false);
      },
      (err) => {
        setError(err instanceof Error ? err : new Error(String(err)));
        setLoading(false);
      },
    );

    return unsub;
  }, [elevatorId, subcollectionName, orderField]);

  return { data, loading, error };
}
